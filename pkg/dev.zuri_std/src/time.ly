
@@Plugin("/time")

/**
 *
 */
@AllocatorTrap()
class Timezone {

    init(offset: Int) {
        @{
            Trap()
        }
    }
}

/**
 *
 */
@AllocatorTrap()
class Instant {

    init() {
        @{
            Trap()
        }
    }

    def ToEpochMillis(): Int {
        @{
            Trap()
            PushResult(typeof Int)
        }
    }
}

/**
 *
 */
@AllocatorTrap()
defclass Datetime {

    init(instant: Instant, timezone: Timezone) {
        @{
            Trap()
        }
    }
}

/**
 *
 */
def Now(): Instant {
    @{
        Trap()
        PushResult(typeof Instant)
    }
}

/**
 *
 */
def ParseTimezone(name: String): Timezone {
    @{
        Trap()
        PushResult(typeof Timezone)
    }
}

