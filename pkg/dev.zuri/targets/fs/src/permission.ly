
import from "//std/flags" { Flags, IntoFlags }

defenum Permission {

    val Value: Int

    init(value: Int) {
        set this.Value = value
    }

    case UserRead(256)
    case UserWrite(128)
    case UserExec(64)
    case GroupRead(32)
    case GroupWrite(16)
    case GroupExec(8)
    case OtherRead(4)
    case OtherWrite(2)
    case OtherExec(1)
}

definstance PermissionInstance {
    impl IntoFlags[Permission] {
        def ToValue(flag: Permission): Int {
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
        set this.Mode = perms
    }

    init Default() from this(UserRead, UserWrite, GroupRead, GroupWrite, OtherRead, OtherWrite) {
    }
}