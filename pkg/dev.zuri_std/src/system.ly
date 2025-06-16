
@@Plugin("/system")


/*
 *
 */

@AllocatorTrap("STD_SYSTEM_FUTURE_ALLOC")
defclass Future[T] final {

    init() {
        @{
            Trap("STD_SYSTEM_FUTURE_CTOR")
        }
    }

    def Complete(result: T): Bool {
        @{
            Trap("STD_SYSTEM_FUTURE_COMPLETE")
            PushResult(typeof Bool)
        }
    }

    def Reject(status: Status): Bool {
        @{
            Trap("STD_SYSTEM_FUTURE_REJECT")
            PushResult(typeof Bool)
        }
    }

    def Cancel(): Bool {
        val cancelled: Cancelled = Cancelled{message = "cancelled"}
        this.Reject(cancelled)
    }

    def Then[U](fun: Function1[U|Status, T|Status]): Future[U] {
        @{
            Trap("STD_SYSTEM_FUTURE_THEN")
            PushResult(typeof Future[U])
        }
    }
}

@AllocatorTrap("STD_SYSTEM_QUEUE_ALLOC")
defclass Queue[T] {

    def Push(element: T): Bool {
        @{
            Trap("STD_SYSTEM_QUEUE_PUSH")
            PushResult(typeof Bool)
        }
    }

    def Pop(): Future[T] {
        @{
            Trap("STD_SYSTEM_QUEUE_POP")
            PushResult(typeof Future[T])
        }
    }
}

defstruct Message {
    val SeqNr: Int
    val Payload: Bytes
}

@AllocatorTrap("STD_SYSTEM_PORT_ALLOC")
defclass Port sealed {

    def Send(payload: Bytes): Message | Status {
        @{
            Trap("STD_SYSTEM_PORT_SEND")
            PushResult(typeof Message | Status)
        }
    }

    def Receive(): Future[Message] {
        @{
            Trap("STD_SYSTEM_PORT_RECEIVE")
            PushResult(typeof Future[Message])
        }
    }
}

def Acquire(url: Url): Port {
    @{
        Trap("STD_SYSTEM_ACQUIRE")
        PushResult(typeof Port)
    }
}

def Await[T](fut: Future[T]): T | Status {
    @{
        Trap("STD_SYSTEM_AWAIT")
        Trap("STD_SYSTEM_GET_RESULT")
        PushResult(typeof T | Status)
    }
}

def AwaitOrDefault[T](fut: Future[T], default: T): T {
    var result: T | Status = default
    @{
        Trap("STD_SYSTEM_AWAIT")
        Trap("STD_SYSTEM_GET_RESULT")
        PushResult(typeof T | Status)
        StoreData(result)
    }
    match result {
        when status: Status     default
        else                    result
    }
}

def Sleep(millis: Int): Future[Undef] {
    @{
        Trap("STD_SYSTEM_SLEEP")
        PushResult(typeof Future[Undef])
    }
}

def Spawn[T](fun: Function0[T]): Future[T] {
    @{
        Trap("STD_SYSTEM_SPAWN")
        PushResult(typeof Future[T])
    }
}