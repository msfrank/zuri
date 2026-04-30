
@@Plugin("/system")

def ResourceExists(path: String): Bool {
    @{
        Trap("STD_RESOURCE_EXISTS")
        PushResult(typeof Bool)
    }
}

def LoadResource(path: String): Bytes | Status {
    @{
        Trap("STD_RESOURCE_LOAD")
        PushResult(typeof Bytes | Status)
    }
}