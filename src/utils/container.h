#pragma once
#pragma GCC diagnostic ignored "-Wpedantic"

#include <assert.h>
#include <stdlib.h>

#define ZAZ_CAT_(a, b) a##b
#define ZAZ_CAT(a, b) ZAZ_CAT_(a, b)

// ============================================================================
// 								VECTOR
// ============================================================================

#define VECTOR(TYPE) ZAZ_CAT(TYPE, _vector)

#define VECTOR_DECLARE(TYPE)                                                   \
    typedef struct VECTOR(TYPE)                                                \
    {                                                                          \
        TYPE* data;                                                            \
        size_t size;                                                           \
        size_t capacity;                                                       \
    } VECTOR(TYPE)

#define VECTOR_EMPTY(VEC) ((VEC).size == 0)
#define VECTOR_INIT(VEC) (VEC) = (typeof(VEC)){0}
#define VECTOR_BEGIN(VEC) (VEC).data
#define VECTOR_END(VEC) (VEC).data + (VEC).size
#define VECTOR_NEXT(ITR) (ITR) + 1
#define VECTOR_SIZE(VEC) (VEC).size
#define VECTOR_GET(VEC, INDEX) (VEC).data[(INDEX)]
#define VECTOR_TYPE(VEC) typeof(*(VEC).data)

#define VECTOR_FOR_EACH(VEC, ITR)                                              \
    for (auto ITR = VECTOR_BEGIN(VEC); ITR != VECTOR_END(VEC);                 \
         ITR = VECTOR_NEXT(ITR))

#define VECTOR_POP_BACK(VEC)                                                   \
    do                                                                         \
    {                                                                          \
        if ((VEC).size)                                                        \
            ((VEC).size)--;                                                    \
    } while (0)

#define VECTOR_FREE(VEC)                                                       \
    do                                                                         \
    {                                                                          \
        free((VEC).data);                                                      \
        (VEC) = ((typeof(VEC)){0});                                            \
    } while (0)

#define VECTOR_PUSH_BACK(VEC, VAL)                                             \
    ({                                                                         \
        typeof(VEC)* _vp = &(VEC);                                             \
        typeof(*_vp->data) _tmp = (VAL);                                       \
        bool _ok = true;                                                       \
                                                                               \
        if (_vp->size >= _vp->capacity)                                        \
        {                                                                      \
            size_t _new_cap = _vp->capacity ? _vp->capacity * 2 : 2;           \
            void* _new = realloc(_vp->data, _new_cap * sizeof(*_vp->data));    \
            if (_new == NULL)                                                  \
                _ok = false;                                                   \
            else                                                               \
            {                                                                  \
                _vp->data = _new;                                              \
                _vp->capacity = _new_cap;                                      \
            }                                                                  \
        }                                                                      \
                                                                               \
        if (_ok)                                                               \
            _vp->data[_vp->size++] = _tmp;                                     \
        _ok;                                                                   \
    })

#define VECTOR_FIND(VEC, VAL)                                                  \
    ({                                                                         \
        typeof(VEC)* _vp = &(VEC);                                             \
        static_assert(                                                         \
            __builtin_types_compatible_p(typeof(*_vp->data), typeof(VAL)),     \
            "Type mismatch");                                                  \
                                                                               \
        typeof(_vp->data) _res = NULL;                                         \
        for (size_t i = 0; i < _vp->size; ++i)                                 \
            if (_vp->data[i] == (VAL))                                         \
            {                                                                  \
                _res = &_vp->data[i];                                          \
                break;                                                         \
            }                                                                  \
        _res;                                                                  \
    })

#define VECTOR_CONTAINS(VEC, VAL) (VECTOR_FIND((VEC), (VAL)) != NULL)

#define VECTOR_FIND_PREDICATE(VEC, ELEM, PREDICATE)                            \
    ({                                                                         \
        typeof(VEC)* _vfp_vp = &(VEC);                                         \
        VECTOR_TYPE(*_vfp_vp)* _vfp_result = NULL;                             \
        for (size_t _vfp_i = 0; _vfp_i < _vfp_vp->size; _vfp_i++)              \
        {                                                                      \
            VECTOR_TYPE(*_vfp_vp) ELEM = _vfp_vp->data[_vfp_i];                \
            if (PREDICATE)                                                     \
            {                                                                  \
                _vfp_result = _vfp_vp->data + _vfp_i;                          \
                break;                                                         \
            }                                                                  \
        }                                                                      \
        _vfp_result;                                                           \
    })

// ============================================================================
// 								SET
// ============================================================================

#define SET(TYPE) ZAZ_CAT(TYPE, _set)

#define SET_BASE_CAPACITY 8
#define SET_RESIZE_THRESHOLD 0.75f

#define _SET_INSERT_FN(TYPE) ZAZ_CAT(__set_insert_, TYPE)
#define _SET_CONTAINS_FN(TYPE) ZAZ_CAT(__set_contains_, TYPE)
#define _SET_REMOVE_FN(TYPE) ZAZ_CAT(__set_remove_, TYPE)
#define _SET_RESIZE_FN(TYPE) ZAZ_CAT(__set_resize_, TYPE)

#define SET_DECLARE(TYPE, HASH, EQ)                                            \
    VECTOR_DECLARE(TYPE);                                                      \
                                                                               \
    typedef struct SET(TYPE)                                                   \
    {                                                                          \
        size_t size;                                                           \
        size_t capacity;                                                       \
        VECTOR(TYPE) * buckets;                                                \
    } SET(TYPE);                                                               \
                                                                               \
    static inline void _SET_RESIZE_FN(TYPE)(SET(TYPE) * s)                     \
    {                                                                          \
        size_t oldc = s->capacity;                                             \
        VECTOR(TYPE)* oldb = s->buckets;                                       \
                                                                               \
        s->capacity = s->capacity ? s->capacity * 2 : SET_BASE_CAPACITY;       \
        s->buckets = malloc(s->capacity * sizeof(*s->buckets));                \
        assert(s->buckets != NULL);                                            \
        s->size = 0;                                                           \
        for (size_t i = 0; i < s->capacity; ++i)                               \
            s->buckets[i] = (VECTOR(TYPE)){0};                                 \
                                                                               \
        for (size_t i = 0; i < oldc; ++i)                                      \
        {                                                                      \
            VECTOR(TYPE)* ob = &oldb[i];                                       \
            for (size_t j = 0; j < ob->size; ++j)                              \
            {                                                                  \
                size_t ix = HASH(ob->data[j]) % s->capacity;                   \
                VECTOR_PUSH_BACK(s->buckets[ix], ob->data[j]);                 \
                s->size++;                                                     \
            }                                                                  \
            VECTOR_FREE(*ob);                                                  \
        }                                                                      \
        free(oldb);                                                            \
    }                                                                          \
                                                                               \
    static inline bool _SET_CONTAINS_FN(TYPE)(SET(TYPE) * s, TYPE v)           \
    {                                                                          \
        if (!s->capacity)                                                      \
            return false;                                                      \
        size_t idx = HASH(v) % s->capacity;                                    \
        VECTOR(TYPE)* b = &s->buckets[idx];                                    \
        for (size_t j = 0; j < b->size; ++j)                                   \
            if (EQ(b->data[j], v))                                             \
                return true;                                                   \
        return false;                                                          \
    }                                                                          \
                                                                               \
    static inline bool _SET_INSERT_FN(TYPE)(SET(TYPE) * s, TYPE v)             \
    {                                                                          \
        if (_SET_CONTAINS_FN(TYPE)(s, v))                                      \
            return false;                                                      \
        if (!s->capacity                                                       \
            || (float)s->size / (float)s->capacity >= SET_RESIZE_THRESHOLD)    \
            _SET_RESIZE_FN(TYPE)(s);                                           \
        size_t idx = HASH(v) % s->capacity;                                    \
        bool ok = VECTOR_PUSH_BACK(s->buckets[idx], v);                        \
        if (ok)                                                                \
            s->size++;                                                         \
        return ok;                                                             \
    }                                                                          \
                                                                               \
    static inline bool _SET_REMOVE_FN(TYPE)(SET(TYPE) * s, TYPE v)             \
    {                                                                          \
        if (!s->capacity)                                                      \
            return false;                                                      \
        size_t idx = HASH(v) % s->capacity;                                    \
        VECTOR(TYPE)* b = &s->buckets[idx];                                    \
        for (size_t j = 0; j < b->size; ++j)                                   \
        {                                                                      \
            if (EQ(b->data[j], v))                                             \
            {                                                                  \
                if (j != b->size - 1)                                          \
                    b->data[j] = b->data[b->size - 1];                         \
                b->size--;                                                     \
                s->size--;                                                     \
                return true;                                                   \
            }                                                                  \
        }                                                                      \
        return false;                                                          \
    }

#define SET_INIT() {0}

#define SET_TYPE(S) typeof(*(S).buckets[0].data)
#define SET_SIZE(S) (S).size
#define SET_EMPTY(S) ((S).size == 0)

#define SET_FOR_EACH(S, ITR)                                                   \
    for (size_t _i = 0; _i < (S).capacity; ++_i)                               \
        for (auto ITR = VECTOR_BEGIN((S).buckets[_i]);                         \
             ITR != VECTOR_END((S).buckets[_i]); ITR = VECTOR_NEXT(ITR))

#define SET_CONTAINS_T(TYPE, S, V) _SET_CONTAINS_FN(TYPE)(&(S), (V))

#define SET_INSERT_T(TYPE, S, V) _SET_INSERT_FN(TYPE)(&(S), (V))

#define SET_REMOVE_T(TYPE, S, V) _SET_REMOVE_FN(TYPE)(&(S), (V))

#define SET_RESIZE_T(TYPE, SPOINTER) _SET_RESIZE_FN(TYPE)(SPOINTER)

#define SET_FREE(S)                                                            \
    do                                                                         \
    {                                                                          \
        typeof(S)* _sp = &(S);                                                 \
        for (size_t _i = 0; _i < _sp->capacity; ++_i)                          \
            VECTOR_FREE(_sp->buckets[_i]);                                     \
        free(_sp->buckets);                                                    \
        _sp->buckets = NULL;                                                   \
        _sp->size = 0;                                                         \
        _sp->capacity = 0;                                                     \
    } while (0)

#define SET_FIND_PREDICATE(S, ELEM, PREDICATE)                                 \
    ({                                                                         \
        SET_TYPE(S)* _result = NULL;                                           \
        for (size_t _i = 0; _i < (S).capacity && !_result; ++_i)               \
            _result = VECTOR_FIND_PREDICATE((S).buckets[_i], ELEM, PREDICATE); \
        _result;                                                               \
    })

#define SET_CONTAINS_PREDICATE(S, ELEM, PREDICATE)                             \
    (SET_FIND_PREDICATE((S), ELEM, (PREDICATE)) != NULL)

// ============================================================================
// 								MAP
// ============================================================================

#define MAP(KT, VT) ZAZ_CAT(ZAZ_CAT(KT, _), ZAZ_CAT(VT, _map))
#define _MAP_ENTRY(KT, VT) ZAZ_CAT(MAP(KT, VT), _entry)

#define MAP_BASE_CAPACITY 8
#define MAP_RESIZE_THRESHOLD 0.75f

#define _MAP_INSERT_FN(KT, VT)                                                 \
    ZAZ_CAT(__map_insert_, ZAZ_CAT(KT, ZAZ_CAT(_, VT)))
#define _MAP_CONTAINS_FN(KT, VT)                                               \
    ZAZ_CAT(__map_contains_, ZAZ_CAT(KT, ZAZ_CAT(_, VT)))
#define _MAP_GET_FN(KT, VT) ZAZ_CAT(__map_get_, ZAZ_CAT(KT, ZAZ_CAT(_, VT)))
#define _MAP_REMOVE_FN(KT, VT)                                                 \
    ZAZ_CAT(__map_remove_, ZAZ_CAT(KT, ZAZ_CAT(_, VT)))
#define _MAP_SET_FN(KT, VT) ZAZ_CAT(__map_set_, ZAZ_CAT(KT, ZAZ_CAT(_, VT)))
#define _MAP_RESIZE_FN(KT, VT)                                                 \
    ZAZ_CAT(__map_resize_, ZAZ_CAT(KT, ZAZ_CAT(_, VT)))

#define MAP_DECLARE(KT, VT, HASH, EQ)                                          \
    typedef struct _MAP_ENTRY(KT, VT)                                          \
    {                                                                          \
        KT key;                                                                \
        VT value;                                                              \
    } _MAP_ENTRY(KT, VT);                                                      \
                                                                               \
    VECTOR_DECLARE(_MAP_ENTRY(KT, VT));                                        \
                                                                               \
    typedef struct MAP(KT, VT)                                                 \
    {                                                                          \
        size_t size;                                                           \
        size_t capacity;                                                       \
        VECTOR(_MAP_ENTRY(KT, VT)) * buckets;                                  \
    } MAP(KT, VT);                                                             \
                                                                               \
    static inline void _MAP_RESIZE_FN(KT, VT)(MAP(KT, VT) * m)                 \
    {                                                                          \
        size_t oldc = m->capacity;                                             \
        VECTOR(_MAP_ENTRY(KT, VT))* oldb = m->buckets;                         \
                                                                               \
        m->capacity = m->capacity ? m->capacity * 2 : MAP_BASE_CAPACITY;       \
        m->buckets = malloc(m->capacity * sizeof(*m->buckets));                \
        assert(m->buckets != NULL);                                            \
        m->size = 0;                                                           \
        for (size_t i = 0; i < m->capacity; ++i)                               \
            m->buckets[i] = (VECTOR(_MAP_ENTRY(KT, VT))){0};                   \
                                                                               \
        for (size_t i = 0; i < oldc; ++i)                                      \
        {                                                                      \
            VECTOR(_MAP_ENTRY(KT, VT))* ob = &oldb[i];                         \
            for (size_t j = 0; j < ob->size; ++j)                              \
            {                                                                  \
                size_t ix = HASH(ob->data[j].key) % m->capacity;               \
                VECTOR_PUSH_BACK(m->buckets[ix], ob->data[j]);                 \
                m->size++;                                                     \
            }                                                                  \
            VECTOR_FREE(*ob);                                                  \
        }                                                                      \
        free(oldb);                                                            \
    }                                                                          \
                                                                               \
    static inline bool _MAP_CONTAINS_FN(KT, VT)(MAP(KT, VT) * m, KT k)         \
    {                                                                          \
        if (!m->capacity)                                                      \
            return false;                                                      \
                                                                               \
        size_t idx = HASH(k) % m->capacity;                                    \
        VECTOR(_MAP_ENTRY(KT, VT))* b = &m->buckets[idx];                      \
        for (size_t j = 0; j < b->size; ++j)                                   \
            if (EQ(b->data[j].key, k))                                         \
                return true;                                                   \
        return false;                                                          \
    }                                                                          \
                                                                               \
    static inline VT* _MAP_GET_FN(KT, VT)(MAP(KT, VT) * m, KT k)               \
    {                                                                          \
        if (!m->capacity)                                                      \
            return NULL;                                                       \
        size_t idx = HASH(k) % m->capacity;                                    \
        VECTOR(_MAP_ENTRY(KT, VT))* b = &m->buckets[idx];                      \
        for (size_t j = 0; j < b->size; ++j)                                   \
            if (EQ(b->data[j].key, k))                                         \
                return &b->data[j].value;                                      \
        return NULL;                                                           \
    }                                                                          \
                                                                               \
    static inline bool _MAP_INSERT_FN(KT, VT)(MAP(KT, VT) * m, KT k, VT v)     \
    {                                                                          \
        if (_MAP_CONTAINS_FN(KT, VT)(m, k))                                    \
            return false;                                                      \
        if (!m->capacity                                                       \
            || (float)m->size / (float)m->capacity >= MAP_RESIZE_THRESHOLD)    \
            _MAP_RESIZE_FN(KT, VT)(m);                                         \
        size_t idx = HASH(k) % m->capacity;                                    \
        _MAP_ENTRY(KT, VT) entry = {.key = k, .value = v};                     \
        bool ok = VECTOR_PUSH_BACK(m->buckets[idx], entry);                    \
        if (ok)                                                                \
            m->size++;                                                         \
        return ok;                                                             \
    }                                                                          \
                                                                               \
    static inline bool _MAP_SET_FN(KT, VT)(MAP(KT, VT) * m, KT k, VT v)        \
    {                                                                          \
        VT* existing = _MAP_GET_FN(KT, VT)(m, k);                              \
        if (existing != NULL)                                                  \
        {                                                                      \
            *existing = v;                                                     \
            return false;                                                      \
        }                                                                      \
        _MAP_INSERT_FN(KT, VT)(m, k, v);                                       \
        return true;                                                           \
    }                                                                          \
                                                                               \
    static inline bool _MAP_REMOVE_FN(KT, VT)(MAP(KT, VT) * m, KT k)           \
    {                                                                          \
        if (!m->capacity)                                                      \
            return false;                                                      \
        size_t idx = HASH(k) % m->capacity;                                    \
        VECTOR(_MAP_ENTRY(KT, VT))* b = &m->buckets[idx];                      \
        for (size_t j = 0; j < b->size; ++j)                                   \
        {                                                                      \
            if (EQ(b->data[j].key, k))                                         \
            {                                                                  \
                if (j != b->size - 1)                                          \
                    b->data[j] = b->data[b->size - 1];                         \
                b->size--;                                                     \
                m->size--;                                                     \
                return true;                                                   \
            }                                                                  \
        }                                                                      \
        return false;                                                          \
    }

#define MAP_INIT() {0}

#define MAP_KEY_TYPE(M) typeof((M).buckets[0].data[0].key)
#define MAP_VALUE_TYPE(M) typeof((M).buckets[0].data[0].value)
#define MAP_ENTRY_TYPE(M) typeof(*(M).buckets[0].data)
#define MAP_SIZE(M) (M).size
#define MAP_EMPTY(M) ((M).size == 0)

#define MAP_FOR_EACH(M, ITR)                                                   \
    for (size_t _i = 0; _i < (M).capacity; ++_i)                               \
        for (auto ITR = VECTOR_BEGIN((M).buckets[_i]);                         \
             ITR != VECTOR_END((M).buckets[_i]); ITR = VECTOR_NEXT(ITR))

#define MAP_CONTAINS_T(KT, VT, M, K) _MAP_CONTAINS_FN(KT, VT)(&(M), (K))

#define MAP_GET_T(KT, VT, M, K) _MAP_GET_FN(KT, VT)(&(M), (K))

#define MAP_INSERT_T(KT, VT, M, K, V) _MAP_INSERT_FN(KT, VT)(&(M), (K), (V))

#define MAP_SET_T(KT, VT, M, K, V) _MAP_SET_FN(KT, VT)(&(M), (K), (V))

#define MAP_REMOVE_T(KT, VT, M, K) _MAP_REMOVE_FN(KT, VT)(&(M), (K))

#define MAP_RESIZE_T(KT, VT, MPOINTER) _MAP_RESIZE_FN(KT, VT)(MPOINTER)

#define MAP_FREE(M)                                                            \
    do                                                                         \
    {                                                                          \
        typeof(M)* _mp = &(M);                                                 \
        for (size_t _i = 0; _i < _mp->capacity; ++_i)                          \
            VECTOR_FREE(_mp->buckets[_i]);                                     \
        free(_mp->buckets);                                                    \
        _mp->buckets = NULL;                                                   \
        _mp->size = 0;                                                         \
        _mp->capacity = 0;                                                     \
    } while (0)

#define MAP_FIND_PREDICATE(M, ELEM, PREDICATE)                                 \
    ({                                                                         \
        MAP_ENTRY_TYPE(M)* _result = NULL;                                     \
        for (size_t _i = 0; _i < (M).capacity && !_result; ++_i)               \
            _result = VECTOR_FIND_PREDICATE((M).buckets[_i], ELEM, PREDICATE); \
        _result;                                                               \
    })

#define MAP_CONTAINS_PREDICATE(M, ELEM, PREDICATE)                             \
    (MAP_FIND_PREDICATE((M), ELEM, (PREDICATE)) != NULL)

// ============================================================================
// 								DLIST
// ============================================================================

#define DLIST_NODE(type) type##_dlist_node
#define DLIST(type) type##_dlist

#define DLIST_DECLARE(TYPE)                                                    \
    typedef struct DLIST_NODE(TYPE)                                            \
    {                                                                          \
        TYPE data;                                                             \
        struct DLIST_NODE(TYPE) * next;                                        \
        struct DLIST_NODE(TYPE) * prev;                                        \
    } DLIST_NODE(TYPE);                                                        \
    typedef struct DLIST(TYPE)                                                 \
    {                                                                          \
        DLIST_NODE(TYPE) * head;                                               \
        DLIST_NODE(TYPE) * tail;                                               \
        size_t size;                                                           \
    } DLIST(TYPE)

#define DLIST_INIT(list) (list) = (typeof(list)){0}

#define DLIST_EMPTY(list) ((list).head == NULL)

#define DLIST_PUSH_FRONT(list, value)                                          \
    do                                                                         \
    {                                                                          \
        typeof(*(list).head)* _node = malloc(sizeof(*_node));                  \
        _node->data = (value);                                                 \
        _node->prev = NULL;                                                    \
        _node->next = (list).head;                                             \
        if ((list).head)                                                       \
            (list).head->prev = _node;                                         \
        else                                                                   \
            (list).tail = _node;                                               \
        (list).head = _node;                                                   \
        (list).size++;                                                         \
    } while (0)

#define DLIST_PUSH_BACK(list, value)                                           \
    do                                                                         \
    {                                                                          \
        typeof(*(list).tail)* _node = malloc(sizeof(*_node));                  \
        _node->data = (value);                                                 \
        _node->next = NULL;                                                    \
        _node->prev = (list).tail;                                             \
        if ((list).tail)                                                       \
            (list).tail->next = _node;                                         \
        else                                                                   \
            (list).head = _node;                                               \
        (list).tail = _node;                                                   \
        (list).size++;                                                         \
    } while (0)

#define DLIST_BEGIN(list) (list).head
#define DLIST_END(list) NULL
#define DLIST_NEXT(itr) (itr)->next

#define DLIST_FOR_EACH(list, itr)                                              \
    for (auto itr = DLIST_BEGIN(list); (itr) != DLIST_END(list);               \
         itr = DLIST_NEXT(itr))

#define DLIST_FREE(list)                                                       \
    do                                                                         \
    {                                                                          \
        for (auto _itr = DLIST_BEGIN(list); _itr != DLIST_END(list);)          \
        {                                                                      \
            auto _next = _itr->next;                                           \
            free(_itr);                                                        \
            _itr = _next;                                                      \
        }                                                                      \
        (list).head = NULL;                                                    \
        (list).tail = NULL;                                                    \
        (list).size = 0;                                                       \
    } while (0)

#define DLIST_CONTAINS(list, value)                                            \
    ({                                                                         \
        bool _found = false;                                                   \
        for (auto _n = (list).head; _n; _n = _n->next)                        \
            if (_n->data == (value))                                           \
            {                                                                  \
                _found = true;                                                 \
                break;                                                         \
            }                                                                  \
        _found;                                                                \
    })
