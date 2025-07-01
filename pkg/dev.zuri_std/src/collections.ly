
@@Plugin("/collections")

def _ElementEquals[T](lhs: T, rhs: T, eq: Equality[T,T]): Bool {
    eq.Equals(lhs, rhs)
}

def _ElementCompare[T](lhs: T, rhs: T, ord: Ordered[T]): Bool {
    ord.Compare(lhs, rhs)
}

defclass Option[+T] {

    val Value: T | Nil

    init(value: T | Nil = nil) {
        set this.Value = value
    }

    def IsEmpty(): Bool {
        match this.Value {
            when v: T       false
            else            true
        }
    }

    def Get(): T | Nil {
        this.Value
    }

    def GetOrElse(other: T): T {
        match this.Value {
            when v: T       v
            else            other
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

    def Size(): Int {
        @{
            Trap("STD_COLLECTIONS_HASHMAP_SIZE")
            PushResult(typeof Int)
        }
    }

    def Contains(key: K): Bool {
        @{
            Trap("STD_COLLECTIONS_HASHMAP_CONTAINS")
            PushResult(typeof Bool)
        }
    }

    def Get(key: K): V | Nil {
        @{
            Trap("STD_COLLECTIONS_HASHMAP_GET")
            PushResult(typeof V | Nil)
        }
    }

    def Put(key: K, value: V): V | Nil {
        @{
            Trap("STD_COLLECTIONS_HASHMAP_PUT")
            PushResult(typeof V | Nil)
        }
    }

    def Remove(key: K): V | Nil {
        @{
            Trap("STD_COLLECTIONS_HASHMAP_REMOVE")
            PushResult(typeof V | Nil)
        }
    }

    def Clear() {
        @{
            Trap("STD_COLLECTIONS_HASHMAP_CLEAR")
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

    def Size(): Int {
        @{
            Trap("STD_COLLECTIONS_TREEMAP_SIZE")
            PushResult(typeof Int)
        }
    }

    def Contains(key: K): Bool {
        @{
            Trap("STD_COLLECTIONS_TREEMAP_CONTAINS")
            PushResult(typeof Bool)
        }
    }

    def Get(key: K): V | Nil {
        @{
            Trap("STD_COLLECTIONS_TREEMAP_GET")
            PushResult(typeof V | Nil)
        }
    }

    def Put(key: K, value: V): V | Nil {
        @{
            Trap("STD_COLLECTIONS_TREEMAP_PUT")
            PushResult(typeof V | Nil)
        }
    }

    def Remove(key: K): V | Nil {
        @{
            Trap("STD_COLLECTIONS_TREEMAP_REMOVE")
            PushResult(typeof V | Nil)
        }
    }

    def Clear() {
        @{
            Trap("STD_COLLECTIONS_TREEMAP_CLEAR")
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

    def Size(): Int {
        @{
            Trap("STD_COLLECTIONS_TREESET_SIZE")
            PushResult(typeof Int)
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

    def Replace(value: T): T | Nil {
        @{
            Trap("STD_COLLECTIONS_TREESET_REPLACE")
            PushResult(typeof T | Nil)
        }
    }

    def Clear() {
        @{
            Trap("STD_COLLECTIONS_TREESET_CLEAR")
        }
    }

    impl Iterable[T] {

        def Iterate(): Iterator[T] {
            @{
                LoadData(#TreeSetIterator)
                Trap("STD_COLLECTIONS_TREESET_ITERABLE_ITERATE")
                PushResult(typeof Iterator[T])
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

    def Size(): Int {
        @{
            Trap("STD_COLLECTIONS_VECTOR_SIZE")
            PushResult(typeof Int)
        }
    }

    def At(index: Int): T {
        @{
            Trap("STD_COLLECTIONS_VECTOR_AT")
            PushResult(typeof T)
        }
    }

    def Insert(index: Int, value: T) {
        @{
            Trap("STD_COLLECTIONS_VECTOR_INSERT")
        }
    }

    def Append(value: T) {
        @{
            Trap("STD_COLLECTIONS_VECTOR_APPEND")
        }
    }

    def Replace(index: Int, value: T): T | Nil {
        @{
            Trap("STD_COLLECTIONS_VECTOR_REPLACE")
            PushResult(typeof T | Nil)
        }
    }

    def Remove(index: Int): T | Nil {
        @{
            Trap("STD_COLLECTIONS_VECTOR_REMOVE")
            PushResult(typeof T | Nil)
        }
    }

    def Clear() {
        @{
            Trap("STD_COLLECTIONS_VECTOR_CLEAR")
        }
    }

    impl Iterable[T] {

        def Iterate(): Iterator[T] {
            @{
                LoadData(#VectorIterator)
                Trap("STD_COLLECTIONS_VECTOR_ITERABLE_ITERATE")
                PushResult(typeof Iterator[T])
            }
        }
    }
}
