
@@Plugin("/log")

/**
 *
 */
def Log(message: String): Bool {
    @{
        Trap("STD_LOG_LOG")
        PushResult(typeof Bool)
    }
}