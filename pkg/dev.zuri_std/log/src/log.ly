
/**
 *
 */
def Log(message: String): Bool {
    @{
        Trap(0)
        PushResult(typeof Bool)
    }
}