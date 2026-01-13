
@@Plugin("/file")

import from "//std@zuri.dev/system" { Future }
import from "//std@zuri.dev/flags" { Flags, IntoFlags }
import from "/permission" { Permission, Permissions }

defenum FileMode {

    val CanRead: Bool
    val CanWrite: Bool

    init(read: Bool, write: Bool) {
        set this.CanRead = read
        set this.CanWrite = write
    }

    case ReadOnly(true, false)
    case WriteOnly(false, true)
    case ReadWrite(true, true)
}

/**
 *
 */
@AllocatorTrap("FS_FILE_ALLOC")
defclass File final {

    init(path: String) {
        @{
            Trap("FS_FILE_CTOR")
        }
    }

    def Create(
        mode: FileMode,
        permissions: Permissions = Permissions.Default{},
        named truncate: Bool = false,
        named append: Bool = false
    ): File | Status {
        @{
            LoadData(mode.CanRead)
            PopResult()
            LoadData(mode.CanWrite)
            PopResult()
            LoadData(permissions.Mode.ToInt())
            PopResult()
            Trap("FS_FILE_CREATE")
            PushResult(typeof File | Status)
        }
    }

    def Open(
        mode: FileMode,
        named truncate: Bool = false,
        named append: Bool = false,
        named noFollow: Bool = false
    ): File | Status {
        @{
            LoadData(mode.CanRead)
            PopResult()
            LoadData(mode.CanWrite)
            PopResult()
            Trap("FS_FILE_OPEN")
            PushResult(typeof File | Status)
        }
    }

    def OpenOrCreate(
        mode: FileMode,
        permissions: Permissions = Permissions.Default{},
        named truncate: Bool = false,
        named append: Bool = false,
        named noFollow: Bool = false
    ): File | Status {
        @{
            LoadData(mode.CanRead)
            PopResult()
            LoadData(mode.CanWrite)
            PopResult()
            LoadData(permissions.Mode.ToInt())
            PopResult()
            Trap("FS_FILE_OPEN_OR_CREATE")
            PushResult(typeof File | Status)
        }
    }

    def Read(maxBytes: Int): Future[Bytes] {
        val fut: Future[Bytes] = Future[Bytes]{}
        @{
            LoadData(fut)
            Trap("FS_FILE_READ")
        }
        fut
    }

    def Write(bytes: Bytes, fileOffset: Int = -1): Future[Int] {
        val fut: Future[Int] = Future[Int]{}
        @{
            LoadData(fut)
            Trap("FS_FILE_WRITE")
        }
        fut
    }

    def Close(): Undef | Status {
        @{
            Trap("FS_FILE_CLOSE")
            PushResult(typeof Undef | Status)
        }
    }
}