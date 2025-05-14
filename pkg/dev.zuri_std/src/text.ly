
@@Plugin("/text")

@AllocatorTrap(0)
defclass Text {

    init(s: String) {
        @{
            Trap(1)
        }
    }

    def Length(): Int {
        @{
            Trap(2)
            PushResult(typeof Int)
        }
    }

    def At(index: Int): Char {
        @{
            Trap(3)
            PushResult(typeof Char)
        }
    }
}