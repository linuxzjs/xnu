// Copyright (c) 2021-2022 Apple Inc.  All rights reserved.

#pragma once

#include <mach/time_value.h>
#include <os/base.h>
#include <pthread/pthread.h>
#include <stdbool.h>
#include <stdint.h>

#define ARRAY_COUNT(_a) (sizeof((_a)) / sizeof((_a[0])))

#define REQUIRE_RECOUNT_PMCS \
    T_META_REQUIRES_SYSCTL_EQ("kern.monotonic.supported", 1)
#define REQUIRE_RECOUNT_ENERGY \
    T_META_REQUIRES_SYSTCL_EQ("kern.pervasive_energy", 1)
#define REQUIRE_MULTIPLE_PERF_LEVELS \
    T_META_REQUIRES_SYSCTL_EQ("hw.nperflevels", 2)
#define REQUIRE_EXCLAVES \
    T_META_REQUIRES_SYSCTL_EQ("kern.exclaves_status", 1)
#define SET_THREAD_BIND_BOOTARG \
    T_META_BOOTARGS_SET("enable_skstb=1")

// Enable/disable `T_MAYFAIL` for any expects annotated with
// `T_MAYFAIL_IF_ENABLED`. Defaults to disabled.
void set_expects_may_fail(bool may_fail);

// Returns whether may-fail is currently enabled (for expects
// annotated with `T_MAYFAIL_IF_ENABLED`).
bool expects_may_fail(void);

#define T_MAYFAIL_IF_ENABLED(reason) { if (expects_may_fail()) { T_MAYFAIL_WITH_REASON(reason); } }

// Returns true if the system implicitly tracks CPI.
bool has_cpi(void);

// Returns true if precise user kernel (system) times are being tracked,
// and false otherwise.
bool has_user_system_times(void);

// Returns true if the system can track energy usage.
bool has_energy(void);

// Bind the current thread to the given cluster.
void bind_to_cluster(char type);

// Returns the name of the current scheduler policy.
char *sched_policy_name(void);

// Returns the number of perf-levels on the system.
unsigned int perf_level_count(void);

// Returns the name of the specified perf-level.
const char *perf_level_name(unsigned int perf_level);

// Returns the index of the named perf-level.
unsigned int perf_level_index(const char *name);

// Run temporarily on all perf levels -- must have `SET_THREAD_BIND_BOOTARG`.
void run_on_all_perf_levels(void);

// Run temporarily in exclaves on all perf levels -- must have
// `SET_THREAD_BIND_BOOTARG` and
void run_in_exclaves_on_all_perf_levels(void);

// Return the nanoseconds represented by a Mach time.
uint64_t ns_from_mach(uint64_t mach_time);

// Return the nanoseconds represented by a timeval.
uint64_t ns_from_timeval(struct timeval tv);

// Return the timeval represented by nanoseconds.
struct timeval timeval_from_ns(uint64_t ns);

// Return the nanoseconds represented by a Mach time_value.
uint64_t ns_from_time_value(struct time_value tv);

// Return the Mach time_value represented by nanoseconds.
struct time_value time_value_from_ns(uint64_t ns);

// What an actor should do when it's running.
__enum_decl(role_t, uint32_t, {
	ROLE_NONE,
	ROLE_SPIN,
	ROLE_WAIT,
});

// A thread doing work according to a script.
struct actor {
	pthread_t act_thread;
	role_t act_role;
	void *act_context;
};

struct scene {
	unsigned int scn_actor_count;
	uintptr_t scn_spin_sync;
	void *scn_wait_sync;
	struct actor scn_actors[];
};

// Start `n` threads that follow a given pattern of scripts.
struct scene *scene_start(unsigned int n, role_t *roles);

// Stop and destroy previously-started actors.
void scene_end(struct scene *scene);

// Launch a `T_HELPER_DECL`-based helper.
pid_t launch_helper(char *name);
