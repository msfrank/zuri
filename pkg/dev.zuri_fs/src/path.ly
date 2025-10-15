
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

    def Parent(): Path {
        @{
            Trap("FS_PATH_PARENT")
            PushResult(typeof Path)
        }
    }

    def FileName(): Path {
        @{
            Trap("FS_PATH_FILE_NAME")
            PushResult(typeof Path)
        }
    }

    def FileStem(): Path {
        @{
            Trap("FS_PATH_FILE_STEM")
            PushResult(typeof Path)
        }
    }

    def FileExtension(): String {
        @{
            Trap("FS_PATH_FILE_EXTENSION")
            PushResult(typeof Path)
        }
    }

    def IsAbsolute(): Bool {
        @{
            Trap("FS_PATH_IS_ABSOLUTE")
            PushResult(typeof Bool)
        }
    }

    def IsRelative(): Bool {
        @{
            Trap("FS_PATH_IS_RELATIVE")
            PushResult(typeof Bool)
        }
    }

    def ToString(): String {
        @{
            Trap("FS_PATH_TO_STRING")
            PushResult(typeof String)
        }
    }
}