
@@Plugin("/text")

@AllocatorTrap("STD_TEXT_TEXT_ALLOC")
defclass Text {

    init(s: String) {
        @{
            Trap("STD_TEXT_TEXT_CTOR")
        }
    }

    def Length(): Int {
        @{
            Trap("STD_TEXT_TEXT_LENGTH")
            PushResult(typeof Int)
        }
    }

    def At(index: Int): Char {
        @{
            Trap("STD_TEXT_TEXT_AT")
            PushResult(typeof Char)
        }
    }
}