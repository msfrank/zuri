
@@Plugin("/fs")

import from "//std/system" { Future }
import from "//std/flags" { Flags, IntoFlags }
import from "/permission" { Permission, Permissions }

defenum FileMode {

    val CanRead: Bool
    val CanWrite: Bool

    init(read: Bool, write: Bool) {
        this.CanRead = read
        this.CanWrite = write
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
    ): File | Error {
        @{
            LoadData(mode.CanRead)
            PopResult()
            LoadData(mode.CanWrite)
            PopResult()
            LoadData(permissions.Mode.ToInt())
            PopResult()
            Trap("FS_FILE_CREATE")
            PushResult(typeof File | Error)
        }
    }

    def Open(
        mode: FileMode,
        named truncate: Bool = false,
        named append: Bool = false,
        named noFollow: Bool = false
    ): File | Error {
        @{
            LoadData(mode.CanRead)
            PopResult()
            LoadData(mode.CanWrite)
            PopResult()
            Trap("FS_FILE_OPEN")
            PushResult(typeof File | Error)
        }
    }

    def OpenOrCreate(
        mode: FileMode,
        permissions: Permissions = Permissions.Default{},
        named truncate: Bool = false,
        named append: Bool = false,
        named noFollow: Bool = false
    ): File | Error {
        @{
            LoadData(mode.CanRead)
            PopResult()
            LoadData(mode.CanWrite)
            PopResult()
            LoadData(permissions.Mode.ToInt())
            PopResult()
            Trap("FS_FILE_OPEN_OR_CREATE")
            PushResult(typeof File | Error)
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

    def Close(): Status {
        @{
            Trap("FS_FILE_CLOSE")
            PushResult(typeof Status)
        }
    }
}