// Copyright (C) 2026 Koh Swee Teck Dedrick. All rights reserved.

#pragma once

namespace dk {
	struct Thread {
		u64 v;
	};

	using ThreadFunction = void (void *params);

	struct Mutex {
		u64 v;
	};

	struct RWMutex {
		u64 v;
	};

	struct CondVar {
		u64 v;
	};

	struct Semaphore {
		u64 v;
	};

	auto set_thread_name(String8 name) noexcept -> void;
	auto set_thread_namef(char const *fmt, ...) noexcept -> void;
	auto plt_set_thread_name(String8 name) noexcept -> void;

	auto thread_launch(ThreadFunction *func, void *params) noexcept -> Thread;
	auto thread_join(Thread thread) noexcept -> void;
	auto thread_detach(Thread thread) noexcept -> void;

	auto mutex_alloc() noexcept -> Mutex;
	auto mutex_release(Mutex mutex) noexcept -> void;
	auto mutex_scope_enter(Mutex mutex) noexcept -> void;
	auto mutex_scope_leave(Mutex mutex) noexcept -> void;

	auto rw_mutex_alloc() noexcept -> RWMutex;
	auto rw_mutex_release(RWMutex rw_mutex) noexcept -> void;
	auto rw_mutex_scope_enter_w(RWMutex rw_mutex) noexcept -> void;
	auto rw_mutex_scope_leave_w(RWMutex rw_mutex) noexcept -> void;
	auto rw_mutex_scope_enter_r(RWMutex rw_mutex) noexcept -> void;
	auto rw_mutex_scope_leave_r(RWMutex rw_mutex) noexcept -> void;

	auto cond_var_alloc() noexcept -> CondVar;
	auto cond_var_release(CondVar cond_var) noexcept -> void;
	auto cond_var_wait(CondVar cond_var, Mutex mutex, u64 end_time_us) noexcept -> b8;
	auto cond_var_wait_rw_w(CondVar cond_var, RWMutex rw_mutex, u64 end_time_us) noexcept -> b8;
	auto cond_var_wait_rw_r(CondVar cond_var, RWMutex rw_mutex, u64 end_time_us) noexcept -> b8;
	auto cond_var_signal(CondVar cond_var) noexcept -> void;
	auto cond_var_signal_all(CondVar cond_var) noexcept -> void;

	auto semaphore_alloc(u32 initial_count, u32 max_count, String8 name) noexcept -> Semaphore;
	auto semaphore_release(Semaphore semaphore) noexcept -> void;
	auto semaphore_open(String8 name) noexcept -> Semaphore;
	auto semaphore_close(Semaphore semaphore) noexcept -> void;
	auto semaphore_wait(Semaphore semaphore, u64 end_time_us) noexcept -> b8;
	auto semaphore_signal(Semaphore semaphore) noexcept -> void;
}
