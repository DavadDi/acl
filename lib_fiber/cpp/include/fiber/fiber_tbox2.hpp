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
	fiber_tbox2() : size_(0) {}

	~fiber_tbox2() {}

	/**
	 * ������Ϣ������δ�����ѵ���Ϣ����
	 */
	void clear() {
		tbox_.clear();
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

		// ������������Ϣ����
#if __cplusplus >= 201103L || defined(USE_CPP11)     // Support c++11 ?
		tbox_.emplace_back(std::move(t));
#else
		tbox_.push_back(t);
#endif
		size_++;

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
		return size_;
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
	std::list<T>  tbox_;
	size_t        size_;
	fiber_mutex   mutex_;
	fiber_cond    cond_;

	bool peek_obj(T& t) {
		typename std::list<T>::iterator it = tbox_.begin();
		if (it == tbox_.end()) {
			return false;
		}
#if __cplusplus >= 201103L || defined(USE_CPP11)     // Support c++11 ?
		t = std::move(*it);
#else
		t = *it;
#endif
		tbox_.erase(it);
		size_--;
		return true;
	}
};

} // namespace acl
