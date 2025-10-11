
import from "//std@zuri.dev/flags" { Flags, IntoFlags }

defenum Access {

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

    impl IntoFlags[Access] {
        def ToValue(flag: Access): Int {
            flag.Value
        }
    }
}

defclass AccessMode final {

    val Mode: Flags[Access]

    init(mode: ...Access) {
        using Access
        val flags: Flags[Access] = Flags[Access]{}
        for access: Access in mode {
            flags.Set(access)
        }
        set this.Mode = flags
    }
}