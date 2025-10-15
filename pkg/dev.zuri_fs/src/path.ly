
@@Plugin("/path")

@AllocatorTrap("FS_PATH_ALLOC")
defstruct Path {

    init(str: String) {
        @{
            Trap("FS_PATH_CTOR")
        }
    }

    init FromParts(parts: ...String) {
        @{
            Trap("FS_PATH_CTOR")
        }
    }

    init Current() {
        @{
            Trap("FS_PATH_CTOR")
        }
    }

    def ToString(): String {
        @{
            Trap("FS_PATH_TO_STRING")
            PushResult(typeof String)
        }
    }
}