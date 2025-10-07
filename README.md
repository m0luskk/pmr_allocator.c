# C99 Polymorphic allocator
The wrapper for allocator functions for use it in varios cases requiring generic way for memory allocating.
Unfortunately, it only allows a limited number of allocation and freeing options.

## Usage
Macro `Q_PMR_ALLOCATOR` used for creating polymorphic allocator.
```
Q_PMR_ALLOCATOR([underlying allocator], <allocate function>, <free function>)
```
Underlying allocator argument dont specified if allocator is global (externally stateless, e.g. `malloc`).

### Allocate function macro
Allocate function is passed through another macros:
```c
Q_PMR_STATE_ALLOC_F(F) // void* (f) (void* alloc, size_t n)
Q_PMR_STATE_ALIGNED_ALLOC_F(F) // void* (f) (void* alloc, size_t n, size_t align);

// for stateless allocators:
Q_PMR_GLOBAL_ALLOC_F(F) // void* (f) (size_t n)
Q_PMR_GLOBAL_ALIGNED_ALLOC_F(F) // void* (f) (size_t n, size_t align)
```
### Free function macro
Allocate function is also passed through another macros:
```c
Q_PMR_STATE_FREE_F(F) // void (f) (void* alloc, void* pos)
Q_PMR_GLOBAL_FREE_F(F) // void (f) (void* pos);
```

### Using pmr allocator
```
Q_PMR_ALLOCATE(<pmr allocator>, <size>, [alignment])
Q_PMR_FREE(<pmr allocator>, [position])
```

## Examples
### Global allocator
```c
int main(void) {
  struct q_pmr_allocator a = Q_PMR_ALLOCATOR(Q_PMR_GLOBAL_ALLOC_F(malloc), Q_PMR_GLOBAL_FREE_F(free));

  void* mem = Q_PMR_ALLOCATE(&a, sizeof(int));
  assert(mem != NULL);

  Q_PMR_FREE(&a, mem);
}
```
With alignment argument:
```c
int main(void) {
  struct q_pmr_allocator a = Q_PMR_ALLOCATOR(Q_PMR_GLOBAL_ALIGNED_ALLOC_F(local_alloc), Q_PMR_GLOBAL_FREE_F(local_free));

  const size_t int_align = 4; // since C11: alignof(int)/_Alignof(int)
  void* mem = Q_PMR_ALLOCATE(&a, sizeof(int), 4);

  Q_PMR_FREE(&a, mem);
}
```
### State allocator
```c
int main(void) {
  struct state_allocator a = // ...

  struct q_pmr_allocator pmr_a = Q_PMR_ALLOCATOR(&a, Q_PMR_STATE_ALLOC_F(state_allocator_allocate), Q_PMR_STATE_FREE_F(state_allocator_free));

  void* mem = Q_PMR_ALLOCATE(&pmr_a, sizeof(int));
  
  Q_PMR_FREE(&pmr_a, mem);
}
```
With alignment argument:
```c
int main(void) {
  struct state_allocator a = // ...

  struct q_pmr_allocator pmr_a = Q_PMR_ALLOCATOR(&a, Q_PMR_STATE_ALIGNED_ALLOC_F(state_allocator_allocate_aligned), Q_PMR_STATE_FREE_F(state_allocator_free));

  const size_t int_align = 4;
  void* mem = Q_PMR_ALLOCATE(&pmr_a, sizeof(int), 4);
  
  Q_PMR_FREE(&pmr_a, mem);
}
```
### Free completely (e.g. arena)
```c
int main(void) {
  struct some_arena_allocator arena = // ...

  struct q_pmr_allocator pmr_a = Q_PMR_ALLOCATOR(&arena, Q_PMR_STATE_ALLOC_F(some_arena_allocator_allocate), Q_PMR_GLOBAL_FREE_F(some_arena_allocator_free_all));

  void* mem = Q_PMR_ALLOCATE(&pmr_a, sizeof(int));
  
  Q_PMR_FREE(&pmr_a);
}
```