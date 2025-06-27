
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

    init(using eq: Equality[K,K]) {
        @{
            LoadData(#_ElementEquals)
            Trap("STD_COLLECTIONS_HASHMAP_CTOR")
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

    init(using ord: Ordered[K]) {
        @{
            LoadData(#_ElementCompare)
            Trap("STD_COLLECTIONS_TREEMAP_CTOR")
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
