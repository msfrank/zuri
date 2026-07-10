
@@Plugin("/std")

def _ElementEquals[T](lhs: T, rhs: T, eq: Equality[T,T]): Bool {
    eq.Equals(lhs, rhs)
}

def _ElementCompare[T](lhs: T, rhs: T, ord: Ordered[T]): Bool {
    ord.Compare(lhs, rhs)
}

defclass Option[+T] {

    val Value: T | Undef

    init(value: T | Undef = undef) {
        this.Value = value
    }

    def IsEmpty(): Bool {
        match this.Value {
            when v: T -> false
            else      -> true
        }
    }

    def Get(): T | Undef {
        this.Value
    }

    def GetOrElse(other: T): T {
        match this.Value {
            when v: T -> v
            else      -> other
        }
    }
}

@AllocatorTrap("STD_COLLECTIONS_HASHMAP_ITERATOR_ALLOC")
defclass HashMapIterator[K,V] final {
    def _GetKey(): K {
        @{
            Trap("STD_COLLECTIONS_HASHMAP_ITERATOR_GET_KEY")
            PushResult(typeof K)
        }
    }
    def _GetValue(): V {
        @{
            Trap("STD_COLLECTIONS_HASHMAP_ITERATOR_GET_VALUE")
            PushResult(typeof V)
        }
    }
    impl Iterator[Tuple2[K,V]] {
        def Valid(): Bool {
            @{
                Trap("STD_COLLECTIONS_HASHMAP_ITERATOR_VALID")
                PushResult(typeof Bool)
            }
        }
        def Next(): Tuple2[K,V] {
            val key: K = this._GetKey()
            val value: V = this._GetValue()
            @{
                Trap("STD_COLLECTIONS_HASHMAP_ITERATOR_NEXT")
            }
            Tuple2[K,V]{key, value}
        }
    }
}

@AllocatorTrap("STD_COLLECTIONS_HASHMAP_ALLOC")
defclass HashMap[K,+V] {

    init(using eq: Equality[K,K], entries: ...Tuple2[K,V]) {
        @{
            LoadData(#_ElementEquals)
            Trap("STD_COLLECTIONS_HASHMAP_CTOR")
        }
        for entry: Tuple2[K,V] in entries {
            this.Put(entry.Element0, entry.Element1)
        }
    }

    def Size(): I64 {
        @{
            Trap("STD_COLLECTIONS_HASHMAP_SIZE")
            PushResult(typeof I64)
        }
    }

    def Contains(key: K): Bool {
        @{
            Trap("STD_COLLECTIONS_HASHMAP_CONTAINS")
            PushResult(typeof Bool)
        }
    }

    def Get(key: K): V | Undef {
        @{
            Trap("STD_COLLECTIONS_HASHMAP_GET")
            PushResult(typeof V | Undef)
        }
    }

    def Put(key: K, value: V): V | Undef {
        @{
            Trap("STD_COLLECTIONS_HASHMAP_PUT")
            PushResult(typeof V | Undef)
        }
    }

    def Remove(key: K): V | Undef {
        @{
            Trap("STD_COLLECTIONS_HASHMAP_REMOVE")
            PushResult(typeof V | Undef)
        }
    }

    def Clear() {
        @{
            Trap("STD_COLLECTIONS_HASHMAP_CLEAR")
        }
    }

    impl Iterable[HashMap[K,V]] {
        alias IteratorT using Iterable[1] = Tuple2[K,V]

        def Iterate(source: HashMap[K,V]): Iterator[IteratorT] {
            @{
                LoadData(#HashMapIterator)
                Trap("STD_COLLECTIONS_HASHMAP_ITERABLE_ITERATE")
                PushResult(typeof Iterator[IteratorT])
            }
        }
    }
}

@AllocatorTrap("STD_COLLECTIONS_TREEMAP_ITERATOR_ALLOC")
defclass TreeMapIterator[K,V] final {
    def _GetKey(): K {
        @{
            Trap("STD_COLLECTIONS_TREEMAP_ITERATOR_GET_KEY")
            PushResult(typeof K)
        }
    }
    def _GetValue(): V {
        @{
            Trap("STD_COLLECTIONS_TREEMAP_ITERATOR_GET_VALUE")
            PushResult(typeof V)
        }
    }
    impl Iterator[Tuple2[K,V]] {
        def Valid(): Bool {
            @{
                Trap("STD_COLLECTIONS_TREEMAP_ITERATOR_VALID")
                PushResult(typeof Bool)
            }
        }
        def Next(): Tuple2[K,V] {
            val key: K = this._GetKey()
            val value: V = this._GetValue()
            @{
                Trap("STD_COLLECTIONS_TREEMAP_ITERATOR_NEXT")
            }
            Tuple2[K,V]{key, value}
        }
    }
}

@AllocatorTrap("STD_COLLECTIONS_TREEMAP_ALLOC")
defclass TreeMap[K,+V] {

    init(using ord: Ordered[K], entries: ...Tuple2[K,V]) {
        @{
            LoadData(#_ElementCompare)
            Trap("STD_COLLECTIONS_TREEMAP_CTOR")
        }
        for entry: Tuple2[K,V] in entries {
            this.Put(entry.Element0, entry.Element1)
        }
    }

    def Size(): I64 {
        @{
            Trap("STD_COLLECTIONS_TREEMAP_SIZE")
            PushResult(typeof I64)
        }
    }

    def Contains(key: K): Bool {
        @{
            Trap("STD_COLLECTIONS_TREEMAP_CONTAINS")
            PushResult(typeof Bool)
        }
    }

    def Get(key: K): V | Undef {
        @{
            Trap("STD_COLLECTIONS_TREEMAP_GET")
            PushResult(typeof V | Undef)
        }
    }

    def Put(key: K, value: V): V | Undef {
        @{
            Trap("STD_COLLECTIONS_TREEMAP_PUT")
            PushResult(typeof V | Undef)
        }
    }

    def Remove(key: K): V | Undef {
        @{
            Trap("STD_COLLECTIONS_TREEMAP_REMOVE")
            PushResult(typeof V | Undef)
        }
    }

    def Clear() {
        @{
            Trap("STD_COLLECTIONS_TREEMAP_CLEAR")
        }
    }

    impl Iterable[TreeMap[K,V]] {
        alias IteratorT using Iterable[1] = Tuple2[K,V]

        def Iterate(source: TreeMap[K,V]): Iterator[IteratorT] {
            @{
                LoadData(#TreeMapIterator)
                Trap("STD_COLLECTIONS_TREEMAP_ITERABLE_ITERATE")
                PushResult(typeof Iterator[IteratorT])
            }
        }
    }
}

@AllocatorTrap("STD_COLLECTIONS_TREESET_ITERATOR_ALLOC")
defclass TreeSetIterator[T] {
    impl Iterator[T] {
        def Valid(): Bool {
            @{
                Trap("STD_COLLECTIONS_TREESET_ITERATOR_VALID")
                PushResult(typeof Bool)
            }
        }
        def Next(): T {
            @{
                Trap("STD_COLLECTIONS_TREESET_ITERATOR_NEXT")
                PushResult(typeof T)
            }
        }
    }
}

@AllocatorTrap("STD_COLLECTIONS_TREESET_ALLOC")
defclass TreeSet[+T] {

    init(using ord: Ordered[T], elements: ...T) {
        @{
            LoadData(#_ElementCompare)
            Trap("STD_COLLECTIONS_TREESET_CTOR")
        }
        for element: T in elements {
            this.Add(element)
        }
    }

    def Size(): I64 {
        @{
            Trap("STD_COLLECTIONS_TREESET_SIZE")
            PushResult(typeof I64)
        }
    }

    def Contains(value: T): Bool {
        @{
            Trap("STD_COLLECTIONS_TREESET_CONTAINS")
            PushResult(typeof Bool)
        }
    }

    def Add(value: T): Bool {
        @{
            Trap("STD_COLLECTIONS_TREESET_ADD")
            PushResult(typeof Bool)
        }
    }

    def Remove(value: T): Bool {
        @{
            Trap("STD_COLLECTIONS_TREESET_REMOVE")
            PushResult(typeof Bool)
        }
    }

    def Replace(value: T): T | Undef {
        @{
            Trap("STD_COLLECTIONS_TREESET_REPLACE")
            PushResult(typeof T | Undef)
        }
    }

    def Clear() {
        @{
            Trap("STD_COLLECTIONS_TREESET_CLEAR")
        }
    }

    impl Iterable[TreeSet[T]] {
        alias IteratorT using Iterable[1] = T

        def Iterate(source: TreeSet[T]): Iterator[T] {
            @{
                LoadData(#TreeSetIterator)
                Trap("STD_COLLECTIONS_TREESET_ITERABLE_ITERATE")
                PushResult(typeof Iterator[IteratorT])
            }
        }
    }
}

@AllocatorTrap("STD_COLLECTIONS_VECTOR_ITERATOR_ALLOC")
defclass VectorIterator[T] {
    impl Iterator[T] {
        def Valid(): Bool {
            @{
                Trap("STD_COLLECTIONS_VECTOR_ITERATOR_VALID")
                PushResult(typeof Bool)
            }
        }
        def Next(): T {
            @{
                Trap("STD_COLLECTIONS_VECTOR_ITERATOR_NEXT")
                PushResult(typeof T)
            }
        }
    }
}

@AllocatorTrap("STD_COLLECTIONS_VECTOR_ALLOC")
defclass Vector[+T] {

    init(elements: ...T) {
        @{
            Trap("STD_COLLECTIONS_VECTOR_CTOR")
        }
        for element: T in elements {
            this.Append(element)
        }
    }

    def Size(): I64 {
        @{
            Trap("STD_COLLECTIONS_VECTOR_SIZE")
            PushResult(typeof I64)
        }
    }

    def At(index: I64): T {
        @{
            Trap("STD_COLLECTIONS_VECTOR_AT")
            PushResult(typeof T)
        }
    }

    def Insert(index: I64, value: T) {
        @{
            Trap("STD_COLLECTIONS_VECTOR_INSERT")
        }
    }

    def Append(value: T) {
        @{
            Trap("STD_COLLECTIONS_VECTOR_APPEND")
        }
    }

    def Replace(index: I64, value: T): T | Undef {
        @{
            Trap("STD_COLLECTIONS_VECTOR_REPLACE")
            PushResult(typeof T | Undef)
        }
    }

    def Remove(index: I64): T | Undef {
        @{
            Trap("STD_COLLECTIONS_VECTOR_REMOVE")
            PushResult(typeof T | Undef)
        }
    }

    def Clear() {
        @{
            Trap("STD_COLLECTIONS_VECTOR_CLEAR")
        }
    }

    impl Iterable[Vector[T]] {
        alias IteratorT using Iterable[1] = T

        def Iterate(source: Vector[T]): Iterator[T] {
            @{
                LoadData(#VectorIterator)
                Trap("STD_COLLECTIONS_VECTOR_ITERABLE_ITERATE")
                PushResult(typeof Iterator[IteratorT])
            }
        }
    }
}
