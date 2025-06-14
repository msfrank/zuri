
@@Plugin("/collections")

@AllocatorTrap("STD_COLLECTIONS_HASHMAP_ALLOC")
defclass HashMap[K,+V] {

    val eq: Equality[K]

    init(using eq: Equality[K]) {
        set this.eq = eq
        @{
            PushData(#Equality.equals)
            Trap("COLLECTIONS_HASHMAP_CTOR")
        }
    }

    def Size(): Int {
        @{
            Trap("STD_COLLECTIONS_HASHMAP_SIZE")
            PushResult(typeof Int)
        }
    }

    def Contains(): Bool {
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

    def Clear(): Undef {
        @{
            Trap("STD_COLLECTIONS_HASHMAP_CLEAR")
            PushResult(typeof Undef)
        }
    }
}
