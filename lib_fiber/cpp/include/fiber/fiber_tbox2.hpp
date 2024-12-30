#pragma once
#include "fiber_cpp_define.hpp"
#include <list>
#include <vector>
#include <cstdlib>
#include "fiber_mutex.hpp"
#include "fiber_cond.hpp"

namespace acl {

/**
 * ����Э��֮�䣬�߳�֮���Լ�Э�����߳�֮�����Ϣͨ�ţ�ͨ��Э����������
 * ��Э���¼���ʵ��
 *
 * ʾ����
 *
 * class myobj {
 * public:
 *     myobj() {}
 *     ~myobj() {}
 *
 *     void test() { printf("hello world\r\n"); }
 * };
 *
 * acl::fiber_tbox2<myobj> tbox;
 *
 * void thread_producer() {
 *     myobj o;
 *     tbox.push(o);
 * }
 *
 * void thread_consumer() {
 *     myobj o;

 *     if (tbox.pop(o)) {
 *         o.test();
 *     }
 * }
 */

// The fiber_tbox2 has an object copying process in push/pop which is suitable
// for transfering the object managed by std::shared_ptr.

// The base box2<T> defined in acl_cpp/stdlib/box.hpp, so you must include
// box.hpp first before including fiber_tbox.hpp
template<typename T>
class fiber_tbox2 : public box2<T> {
public:
	/**
	 * ���췽��
	 */
	fiber_tbox2() : capacity_(10000) , off_curr_(0) , off_next_(0) {
		box_ = (T*) malloc(sizeof(T) * capacity_);
	}

	~fiber_tbox2() { free(box_); }

	/**
	 * ������Ϣ������δ�����ѵ���Ϣ����
	 */
	void clear() {
		off_curr_ = off_next_ = 0;
	}

	/**
	 * ������Ϣ����
	 * @param t {T} ��Ϣ����
	 * @param notify_first {bool} ���������Ϊ true�����ڲ��������Ϣ��
	 *  ������֪ͨ�������ʽ����������Ƚ�����֪ͨ��ʽ���� fiber_tbox2 ����
	 *  ���������ڱȽϳ�ʱ���ò�����Ϊ false ��Ч�ʸ��ߣ���� fiber_tbox2
	 *  ������������ڽ϶�(�磺�ȴ��ߵ��� pop ��ֱ������ fiber_tbox2 ����),
	 *  �򱾲���Ӧ����Ϊ true���Ա��� push �߻�û����ȫ����ǰ�� fiber_tbox2
	 *  ������ǰ���ٶ�����ڴ�Ƿ�����
	 * @return {bool}
	 * @override
	 */
	bool push(T t, bool notify_first = true) {
		// �ȼ���
		if (! mutex_.lock()) { abort(); }

		if (off_next_ == capacity_) {
			if (off_curr_ >= 10000) {
#if 1
				size_t n = 0;
				for (size_t i = off_curr_; i < off_next_; i++) {
					box_[n++] = box_[i];
				}
#else
				memmove(box_, box_ + off_curr_,
					(off_next_ - off_curr_) * sizeof(T));
#endif

				off_next_ -= off_curr_;
				off_curr_ = 0;
			} else {
				capacity_ += 10000;
				box_ = (T*) realloc(box_, sizeof(T) * capacity_);
			}
		}
#if __cplusplus >= 201103L || defined(USE_CPP11)
		box_[off_next_++] = std::move(t);
#else
		box_[off_next_++] = t;
#endif

		if (notify_first) {
			if (! cond_.notify()) { abort(); }
			if (! mutex_.unlock()) { abort(); }
		} else {
			if (! mutex_.unlock()) { abort(); }
			if (! cond_.notify()) { abort(); }
		}
		return true;
	}

	/**
	 * ������Ϣ����
	 * @param t {T&} ������ ���� true ʱ��Ž������
	 * @param ms {int} >= 0 ʱ���õȴ���ʱʱ��(���뼶��)��
	 *  ������Զ�ȴ�ֱ��������Ϣ��������
	 * @return {bool} �Ƿ�����Ϣ����
	 * @override
	 */
	bool pop(T& t, int ms = -1) {
		if (! mutex_.lock()) { abort(); }
		while (true) {
			if (peek_obj(t)) {
				if (! mutex_.unlock()) { abort(); }
				return true;
			}

			if (!cond_.wait(mutex_, ms) && ms >= 0) {
				if (! mutex_.unlock()) { abort(); }
				return false;
			}
		}
	}

	//@override
	size_t pop(std::vector<T>& out, size_t max, int ms) {
		size_t n = 0;
		if (! mutex_.lock()) { abort(); }
		while (true) {
			T t;
			if (peek_obj(t)) {
				out.push_back(t);
				n++;
				if (max > 0 && n >= max) {
					if (! mutex_.unlock()) { abort(); }
					return n;
				}
				continue;
			}
			if (n > 0) {
				if (! mutex_.lock()) { abort(); }
				return n;
			}
			if (! cond_.wait(mutex_, ms) && ms >= 0) {
				if (! mutex_.lock()) { abort(); }
				return n;
			}
		}
	}

	/**
	 * ���ص�ǰ��������Ϣ�����е���Ϣ����
	 * @return {size_t}
	 * @override
	 */
	size_t size() const {
		return off_next_ - off_curr_;
	}

	// @override
	bool has_null() const {
		return true;
	}

public:
	void lock() {
		if (! mutex_.lock()) { abort(); }
	}

	void unlock() {
		if (! mutex_.unlock()) { abort(); }
	}

private:
	fiber_tbox2(const fiber_tbox2&) {}
	const fiber_tbox2& operator=(const fiber_tbox2&);

private:
	T*           box_;
	size_t       capacity_;
	size_t       off_curr_;
	size_t       off_next_;
	fiber_mutex  mutex_;
	fiber_cond   cond_;

	bool peek_obj(T& t) {
		if (off_curr_ == off_next_) {
			if (off_curr_ > 0) {
				off_curr_ = off_next_ = 0;
			}
			return false;
		}

#if __cplusplus >= 201103L || defined(USE_CPP11)
		t = std::move(box_[off_curr_++]);
#else
		t = box_[off_curr_++];
#endif
		return true;
	}
};

} // namespace acl
