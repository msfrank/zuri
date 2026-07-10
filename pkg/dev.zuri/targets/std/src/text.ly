
@@Plugin("/text")

@AllocatorTrap("STD_TEXT_TEXT_ALLOC")
defclass Text {

    init(s: String) {
        @{
            Trap("STD_TEXT_TEXT_CTOR")
        }
    }

    def Length(): I64 {
        @{
            Trap("STD_TEXT_TEXT_LENGTH")
            PushResult(typeof I64)
        }
    }

    def At(index: I64): Char | Undef {
        @{
            Trap("STD_TEXT_TEXT_AT")
            PushResult(typeof Char | Undef)
        }
    }
}