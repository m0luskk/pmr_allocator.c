#include <check.h>

#include "../include/q_pmr_allocator.h" 

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

struct state_allocator {
  int offset;
};

void* state_allocator_allocate(void* a, size_t size) {
  struct state_allocator* allocator = a;
  allocator->offset += size;
  return NULL;
}
void* state_allocator_allocate_aligned(void* a, size_t size, size_t align) {
  struct state_allocator* allocator = a;
  allocator->offset += size + align;
  return NULL;
}

void state_allocator_free(void* a, void* mem) {
  (void)mem;
  struct state_allocator* allocator = a;
  allocator->offset -= 1;
}
void state_allocator_free_all(void* a) {
  struct state_allocator* allocator = a;
  allocator->offset = 0;
}

void* global_alloc(size_t size, size_t align) {
  printf("aligned state allocator alloc size %zu align %zu\n", size, align);
  return NULL;
}
void global_free(void* p) {
  printf("local destroy at at %p\n", p);
}

START_TEST(global_aligned) {
  struct q_pmr_allocator a = Q_PMR_ALLOCATOR(Q_PMR_GLOBAL_ALIGNED_ALLOC_F(global_alloc), Q_PMR_GLOBAL_FREE_F(global_free));

  void* mem = Q_PMR_ALLOCATE(&a, sizeof(int), 4);
  ck_assert(mem == NULL);

  Q_PMR_FREE(&a, mem);
}
END_TEST

START_TEST(global) {
  struct q_pmr_allocator a = Q_PMR_ALLOCATOR(Q_PMR_GLOBAL_ALLOC_F(malloc), Q_PMR_GLOBAL_FREE_F(free));

  void* mem = Q_PMR_ALLOCATE(&a, sizeof(int));
  ck_assert(mem != NULL);

  Q_PMR_FREE(&a, mem);
}
END_TEST
START_TEST(state) {
  struct state_allocator a = {0};
  struct q_pmr_allocator pmr_a = Q_PMR_ALLOCATOR(&a, Q_PMR_STATE_ALLOC_F(state_allocator_allocate), Q_PMR_STATE_FREE_F(state_allocator_free));

  void* mem = Q_PMR_ALLOCATE(&pmr_a, sizeof(int));
  ck_assert(a.offset == sizeof(int));
  
  Q_PMR_FREE(&pmr_a, mem);
  ck_assert(a.offset == sizeof(int) - 1);
}
END_TEST
START_TEST(state_aligned) {
  struct state_allocator a = {0};
  struct q_pmr_allocator pmr_a = Q_PMR_ALLOCATOR(&a, Q_PMR_STATE_ALIGNED_ALLOC_F(state_allocator_allocate_aligned), Q_PMR_STATE_FREE_F(state_allocator_free));

  void* mem = Q_PMR_ALLOCATE(&pmr_a, sizeof(int), 4);
  ck_assert(a.offset == sizeof(int) + 4);
  
  Q_PMR_FREE(&pmr_a, mem);
  ck_assert(a.offset == sizeof(int) + 4 - 1);
}
END_TEST
START_TEST(state_free_all) {
  struct state_allocator a = {0};
  struct q_pmr_allocator pmr_a = Q_PMR_ALLOCATOR(&a, Q_PMR_STATE_ALLOC_F(state_allocator_allocate), Q_PMR_GLOBAL_FREE_F(state_allocator_free_all));

  void* mem = Q_PMR_ALLOCATE(&pmr_a, sizeof(int));
  ck_assert(a.offset == 4);
  
  Q_PMR_FREE(&pmr_a);
  ck_assert(a.offset == 0);
}
END_TEST

Suite* thpool_suite(void) {
  Suite* s;
  TCase* tc_core;

  s = suite_create("thpool");

  tc_core = tcase_create("Core");

  tcase_add_test(tc_core, global_aligned);
  tcase_add_test(tc_core, global);
  tcase_add_test(tc_core, state);
  tcase_add_test(tc_core, state_aligned);
  tcase_add_test(tc_core, state_free_all);
  
  suite_add_tcase(s, tc_core);

  return s;
}

int main(void) {
  int failed = 0;
  Suite* s;
  SRunner* sr;

  s = thpool_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_NORMAL);

  failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return ( failed ? EXIT_FAILURE : EXIT_SUCCESS  );
}
