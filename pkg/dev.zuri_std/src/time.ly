
@@Plugin("/time")

/**
 *
 */
@AllocatorTrap("STD_TIME_TIMEZONE_ALLOC")
defclass Timezone {

    init(offset: Int) {
        @{
            Trap("STD_TIME_TIMEZONE_CTOR")
        }
    }
}

/**
 *
 */
@AllocatorTrap("STD_TIME_INSTANT_ALLOC")
defclass Instant {

    init() {
        @{
            Trap("STD_TIME_INSTANT_CTOR")
        }
    }

    def ToEpochMillis(): Int {
        @{
            Trap(STD_TIME_INSTANT_TO_EPOCH_MILLIS")
            PushResult(typeof Int)
        }
    }
}

/**
 *
 */
@AllocatorTrap("STD_TIME_DATETIME_ALLOC")
defclass Datetime {

    init(instant: Instant, timezone: Timezone) {
        @{
            Trap("STD_TIME_DATETIME_CTOR")
        }
    }
}

/**
 *
 */
def Now(): Instant {
    @{
        Trap("STD_TIME_NOW")
        PushResult(typeof Instant)
    }
}

/**
 *
 */
def ParseTimezone(name: String): Timezone {
    @{
        Trap("STD_TIME_PARSE_TIMEZONE")
        PushResult(typeof Timezone)
    }
}

