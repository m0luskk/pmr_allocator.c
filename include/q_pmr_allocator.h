#ifndef _Q_PMR_ALLOCATOR_H
#define _Q_PMR_ALLOCATOR_H

#include <stdlib.h>

#ifdef __GNUC__
#define _Q_NODISCARD_ATTR __attribute__((warn_unused_result))
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 202311L) 
#define _Q_NODISCARD_ATTR [[nodiscard]]
#else
#define _Q_NODISCARD_ATTR
#endif

enum _q_f_type {
  _Q_F_TYPE_STATE,
  _Q_F_TYPE_STATE_ALIGNED,
  _Q_F_TYPE_GLOBAL,
  _Q_F_TYPE_GLOBAL_ALIGNED,
};

typedef void*(*g_alloc_f)(size_t);
typedef void*(*g_algnd_alloc_f)(size_t, size_t);
typedef void*(*s_alloc_f)(void*, size_t);
typedef void*(*s_algnd_alloc_f)(void*, size_t, size_t);

typedef void(*s_free_f)(void*, void*);
typedef void(*g_free_f)(void*);

struct _q_free_f_tagged { 
  union {
    s_free_f _s; 
    g_free_f _g;
  } f; 
  enum _q_f_type t; 
};

struct _q_alloc_f_tagged { 
  union {
    s_alloc_f _s; 
    s_algnd_alloc_f _s_a;
    g_algnd_alloc_f _g_a;
    g_alloc_f _g;
  } f; 
  enum _q_f_type t; 
};

typedef struct q_pmr_allocator {
  void* const _allocator;
  const struct _q_alloc_f_tagged _alloc_f;
  const struct _q_free_f_tagged _free_f;
} q_pmr_allocator_t;

// ---

#define _Q_EMPTY()
#define _Q_DEFER1(m) m _Q_EMPTY()
#define _Q_DEFER2(m) _Q_DEFER1 _Q_EMPTY()  (m) 
#define _Q_DEFER3(m) _Q_DEFER2 _Q_EMPTY() (m)
#define _Q_EXPAND(...) __VA_ARGS__

#define Q_PMR_STATE_FREE_F(F) _Q_DEFER3(_Q_PMR_STATE_FREE_F)(F)
#define _Q_PMR_STATE_FREE_F(F) \
(struct _q_free_f_tagged ) { .f = { ._s = F }, .t = _Q_F_TYPE_STATE }

#define Q_PMR_GLOBAL_FREE_F(F) _Q_DEFER3(_Q_PMR_GLOBAL_FREE_F)(F)
#define _Q_PMR_GLOBAL_FREE_F(F) \
(struct _q_free_f_tagged ) { .f = { ._g = F }, .t = _Q_F_TYPE_GLOBAL }

// ---

#define Q_PMR_STATE_ALLOC_F(F) _Q_DEFER3(_Q_PMR_STATE_ALLOC_F)(F)
#define _Q_PMR_STATE_ALLOC_F(F) \
(struct _q_alloc_f_tagged ) { .f = { ._s = F }, .t = _Q_F_TYPE_STATE }

#define Q_PMR_STATE_ALIGNED_ALLOC_F(F) _Q_DEFER3(_Q_PMR_STATE_ALIGNED_ALLOC_F)(F)
#define _Q_PMR_STATE_ALIGNED_ALLOC_F(F) \
(struct _q_alloc_f_tagged ) { .f = { ._s_a = F }, .t = _Q_F_TYPE_STATE_ALIGNED }

#define Q_PMR_GLOBAL_ALLOC_F(F) _Q_DEFER3(_Q_PMR_GLOBAL_ALLOC_F)(F)
#define _Q_PMR_GLOBAL_ALLOC_F(F) \
(struct _q_alloc_f_tagged ) { .f = { ._g = F }, .t = _Q_F_TYPE_GLOBAL }

#define Q_PMR_GLOBAL_ALIGNED_ALLOC_F(F) _Q_DEFER3(_Q_PMR_GLOBAL_ALIGNED_ALLOC_F)(F)
#define _Q_PMR_GLOBAL_ALIGNED_ALLOC_F(F) \
(struct _q_alloc_f_tagged ) { .f = { ._g_a = F }, .t = _Q_F_TYPE_GLOBAL_ALIGNED }

// ---
_Q_NODISCARD_ATTR
static inline struct q_pmr_allocator 
_q_pmr_state_allocator_wrap(void* allocator, struct _q_alloc_f_tagged alloc_f, struct _q_free_f_tagged free_f) {
  return (struct q_pmr_allocator) { ._allocator = allocator, ._alloc_f = alloc_f, ._free_f = free_f };
}

_Q_NODISCARD_ATTR
static inline struct q_pmr_allocator _q_pmr_allocator_wrap(struct _q_alloc_f_tagged alloc_f, struct _q_free_f_tagged free_f) {
  return _q_pmr_state_allocator_wrap(NULL, alloc_f, free_f);
}

_Q_NODISCARD_ATTR
static inline void* q_pmr_allocator_alloc(struct q_pmr_allocator* pm, size_t n, size_t align) {
  switch (pm->_alloc_f.t) {
    case _Q_F_TYPE_STATE: return pm->_alloc_f.f._s(pm->_allocator, n);
    case _Q_F_TYPE_STATE_ALIGNED: return pm->_alloc_f.f._s_a(pm->_allocator, n, align);
    case _Q_F_TYPE_GLOBAL: return pm->_alloc_f.f._g(n);
    case _Q_F_TYPE_GLOBAL_ALIGNED: return pm->_alloc_f.f._g_a(n, align);
  }
  return NULL;
}

static inline void q_pmr_allocator_free(struct q_pmr_allocator* pm, void* dst) {
  if (pm->_allocator) {
    if (dst) {
      pm->_free_f.f._s(pm->_allocator, dst);
    } else {
      pm->_free_f.f._g(pm->_allocator); // e.g. arena allocator
    }
  } else {
    pm->_free_f.f._g(dst);
  }
}
// #define Q_PM_ALLOCATE(ALLOCATOR, N, ALIGN) q_pm_allocator_alloc((ALLOCATOR), (N))

#define _Q_PMR_ALLOCATOR_GLOBAL(...)  _q_pmr_allocator_wrap(__VA_ARGS__);
#define _Q_PMR_ALLOCATOR_WITH_STATE(...) _q_pmr_state_allocator_wrap(__VA_ARGS__)

#define Q_PMR_ALLOCATOR(...) _Q_PMR_ALLOCATOR(_Q_PMR_ALLOCATOR_HAS_ARGS(__VA_ARGS__) ,__VA_ARGS__)
#define _Q_PMR_ALLOCATOR(...) _Q_PMR_ALLOCATOR2(__VA_ARGS__)
#define _Q_PMR_ALLOCATOR2(has_args, ...) _Q_PMR_ALLOCATOR_ ## has_args (__VA_ARGS__)

#define _Q_PMR_ALLOCATOR_HAS_ARGS(...) _Q_PMR_ALLOCATOR_HAS_ARGS_(__VA_ARGS__, _Q_PMR_ALLOCATOR_HAS_ARGS_SOURCE)
#define _Q_PMR_ALLOCATOR_HAS_ARGS_(...) _Q_PMR_ALLOCATOR_HAS_ARGS_MATCH(__VA_ARGS__)

#define _Q_PMR_ALLOCATOR_HAS_ARGS_MATCH(_1, _2, _3, N, ...) N
#define _Q_PMR_ALLOCATOR_HAS_ARGS_SOURCE WITH_STATE, GLOBAL, ERROR

// ----

#define _Q_PMR_ALLOCATE_ALIGNED(...)  q_pmr_allocator_alloc(__VA_ARGS__);
#define _Q_PMR_ALLOCATE_NALIGNED(...) q_pmr_allocator_alloc(__VA_ARGS__, 1)

#define Q_PMR_ALLOCATE(...) _Q_PMR_ALLOCATE(_Q_PMR_ALLOCATE_HAS_ARGS(__VA_ARGS__) ,__VA_ARGS__)
#define _Q_PMR_ALLOCATE(...) _Q_PMR_ALLOCATE2(__VA_ARGS__)
#define _Q_PMR_ALLOCATE2(has_args, ...) _Q_PMR_ALLOCATE_ ## has_args (__VA_ARGS__)

#define _Q_PMR_ALLOCATE_HAS_ARGS(...) _Q_PMR_ALLOCATE_HAS_ARGS_(__VA_ARGS__, _Q_PMR_ALLOCATE_HAS_ARGS_SOURCE)
#define _Q_PMR_ALLOCATE_HAS_ARGS_(...) _Q_PMR_ALLOCATE_HAS_ARGS_MATCH(__VA_ARGS__)

#define _Q_PMR_ALLOCATE_HAS_ARGS_MATCH(_1, _2, _3, N, ...) N
#define _Q_PMR_ALLOCATE_HAS_ARGS_SOURCE ALIGNED, NALIGNED, ERROR

//

#define _Q_PMR_FREE_GLOBAL(...)  q_pmr_allocator_free(__VA_ARGS__, NULL);
#define _Q_PMR_FREE_WITH_STATE(...) q_pmr_allocator_free(__VA_ARGS__)

#define Q_PMR_FREE(...) _Q_PMR_FREE(_Q_PMR_FREE_HAS_ARGS(__VA_ARGS__) ,__VA_ARGS__)
#define _Q_PMR_FREE(...) _Q_PMR_FREE2(__VA_ARGS__)
#define _Q_PMR_FREE2(has_args, ...) _Q_PMR_FREE_ ## has_args (__VA_ARGS__)

#define _Q_PMR_FREE_HAS_ARGS(...) _Q_PMR_FREE_HAS_ARGS_(__VA_ARGS__, _Q_PMR_FREE_HAS_ARGS_SOURCE)
#define _Q_PMR_FREE_HAS_ARGS_(...) _Q_PMR_FREE_HAS_ARGS_MATCH(__VA_ARGS__)

#define _Q_PMR_FREE_HAS_ARGS_MATCH(_1, _2, N, ...) N
#define _Q_PMR_FREE_HAS_ARGS_SOURCE WITH_STATE, GLOBAL, ERROR

//

#endif