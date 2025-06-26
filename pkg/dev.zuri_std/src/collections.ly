
@@Plugin("/collections")

def _HashMapEquals[T](lhs: T, rhs: T, eq: Equality[T,T]): Bool {
    eq.Equals(lhs, rhs)
}

@AllocatorTrap("STD_COLLECTIONS_HASHMAP_ALLOC")
defclass HashMap[K,+V] {

    init(using eq: Equality[K,K]) {
        @{
            LoadData(#_HashMapEquals)
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
