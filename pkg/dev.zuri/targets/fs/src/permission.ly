
import from "//std/flags" { Flags, IntoFlags }

defenum Permission {

    val Value: U32

    init(value: U32) {
        this.Value = value
    }

    case UserRead(256 as U32)
    case UserWrite(128 as U32)
    case UserExec(64 as U32)
    case GroupRead(32 as U32)
    case GroupWrite(16 as U32)
    case GroupExec(8 as U32)
    case OtherRead(4 as U32)
    case OtherWrite(2 as U32)
    case OtherExec(1 as U32)
}

definstance PermissionInstance {
    impl IntoFlags[Permission] {
        def ToValue(flag: Permission): U32 {
            flag.Value
        }
    }
}

defclass Permissions final {

    val Mode: Flags[Permission]

    init(mode: ...Permission) {
        val perms: Flags[Permission] = Flags[Permission]{intoFlags = PermissionInstance}
        for perm: Permission in mode {
            perms.Set(perm)
        }
        this.Mode = perms
    }

    init Default() from this(
        Permission.UserRead,
        Permission.UserWrite,
        Permission.GroupRead,
        Permission.GroupWrite,
        Permission.OtherRead,
        Permission.OtherWrite) {
    }
}