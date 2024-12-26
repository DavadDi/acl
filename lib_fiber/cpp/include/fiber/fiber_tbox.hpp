#pragma once
#include "fiber_cpp_define.hpp"
#include <list>
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
 * acl::fiber_tbox<myobj> fiber_tbox;
 *
 * void thread_producer() {
 *     myobj* o = new myobj;
 *     fiber_tbox.push(o);
 * }
 *
 * void thread_consumer() {
 *     myobj* o = fiber_tbox.pop();
 *     o->test();
 *     delete o;
 * }
 */

// The base box<T> defined in acl_cpp/stdlib/box.hpp, so you must include
// box.hpp first before including fiber_tbox.hpp
template<typename T>
class fiber_tbox : public box<T> {
public:
	/**
	 * ���췽��
	 * @param free_obj {bool} �� fiber_tbox ����ʱ���Ƿ��Զ���鲢�ͷ�
	 *  δ�����ѵĶ�̬����
	 */
	explicit fiber_tbox(bool free_obj = true) : size_(0), free_obj_(free_obj) {}

	~fiber_tbox() {
		clear(free_obj_);
	}

	/**
	 * ������Ϣ������δ�����ѵ���Ϣ����
	 * @param free_obj {bool} �ͷŵ��� delete ����ɾ����Ϣ����
	 */
	void clear(bool free_obj = false) {
		if (free_obj) {
			for (typename std::list<T*>::iterator it =
				tbox_.begin(); it != tbox_.end(); ++it) {

				delete *it;
			}
		}
		tbox_.clear();
	}

	/**
	 * ������Ϣ����
	 * @param t {T*} �ǿ���Ϣ����
	 * @param notify_first {bool} ���������Ϊ true�����ڲ��������Ϣ��
	 *  ������֪ͨ�������ʽ����������Ƚ�����֪ͨ��ʽ���� fiber_tbox ����
	 *  ���������ڱȽϳ�ʱ���ò�����Ϊ false ��Ч�ʸ��ߣ���� fiber_tbox
	 *  ������������ڽ϶�(�磺�ȴ��ߵ��� pop ��ֱ������ fiber_tbox ����),
	 *  �򱾲���Ӧ����Ϊ true���Ա��� push �߻�û����ȫ����ǰ�� fiber_tbox
	 *  ������ǰ���ٶ�����ڴ�Ƿ�����
	 * @return {bool}
	 * @override
	 */
	bool push(T* t, bool notify_first = true) {
		// �ȼ���
		if (! mutex_.lock()) { abort(); }

		// ������������Ϣ����
		tbox_.push_back(t);
		size_++;

		if (notify_first) {
			if (! cond_.notify()) { abort(); }
			if (! mutex_.unlock()) { abort(); }
			return true;
		} else {
			if (! mutex_.unlock()) { abort(); }
			if (! cond_.notify()) { abort(); }
			return true;
		}
	}

	/**
	 * ������Ϣ����
	 * @param wait_ms {int} >= 0 ʱ���õȴ���ʱʱ��(���뼶��)��
	 *  ������Զ�ȴ�ֱ��������Ϣ��������
	 * @param found {bool*} �ǿ�ʱ��������Ƿ�����һ����Ϣ������Ҫ����
	 *  �������ݿն���ʱ�ļ��
	 * @return {T*} �� NULL ��ʾ���һ����Ϣ���󣬷��� NULL ʱ����Ҫ����һ
	 *  ����飬��������� push ��һ���ն���NULL������������Ҳ���� NULL��
	 *  ����ʱ��Ȼ��Ϊ�����һ����Ϣ����ֻ����Ϊ�ն������ wait_ms ����
	 *  Ϊ -1 ʱ���� NULL ��Ȼ��Ϊ�����һ������Ϣ������� wait_ms ����
	 *  ���� 0 ʱ���� NULL����Ӧ�ü�� found ������ֵΪ true ���� false ��
	 *  �ж��Ƿ�����һ������Ϣ����
	 * @override
	 */
	T* pop(int wait_ms = -1, bool* found = NULL) {
		bool found_flag;
		if (! mutex_.lock()) { abort(); }

		while (true) {
			T* t = peek(found_flag);
			if (found_flag) {
				if (! mutex_.unlock()) { abort(); }
				if (found) {
					*found = found_flag;
				}
				return t;
			}

			// ע�����˳�򣬱����ȵ��� wait ���ж� wait_ms
			if (! cond_.wait(mutex_, wait_ms) && wait_ms >= 0) {
				if (! mutex_.unlock()) { abort(); }
				if (found) {
					*found = false;
				}
				return NULL;
			}
		}
	}

	// @override
	size_t pop(std::vector<T*>& out, size_t max, int milliseconds) {
		size_t n = 0;
		bool found_flag;

		if (! mutex_.lock()) { abort(); }
		while (true) {
			T* t = peek(found_flag);
			if (found_flag) {
				out.push_back(t);
				n++;
				if (max > 0 && n >= max) {
					return n;
				}
				continue;
			}

			if (n > 0) {
				if (! mutex_.unlock()) { abort(); }
				return n;
			}

			if (! cond_.wait(mutex_, milliseconds) && milliseconds >= 0) {
				if (! mutex_.unlock()) { abort(); }
				return n;
			}
		}
	}

	/**
	 * tbox ֧�ִ��ݿ���Ϣ
	 * @return {bool}
	 * @override
	 */
	bool has_null() const {
		return true;
	}

	/**
	 * ���ص�ǰ��������Ϣ�����е���Ϣ����
	 * @return {size_t}
	 */
	size_t size() const {
		return size_;
	}

public:
	void lock() {
		if (! mutex_.lock()) {
			abort();
		}
	}

	void unlock() {
		if (! mutex_.unlock()) {
			abort();
		}
	}

private:
	fiber_tbox(const fiber_tbox&) {}
	const fiber_tbox& operator=(const fiber_tbox&);

private:
	std::list<T*> tbox_;
	size_t        size_;
	bool          free_obj_;
	fiber_mutex   mutex_;
	fiber_cond    cond_;

	T* peek(bool& found_flag) {
		typename std::list<T*>::iterator it = tbox_.begin();
		if (it == tbox_.end()) {
			found_flag = false;
			return NULL;
		}
		found_flag = true;
		size_--;
		T* t = *it;
		tbox_.erase(it);
		return t;
	}
};

} // namespace acl
