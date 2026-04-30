

@@Plugin("/std")


@AllocatorTrap("STD_URL_URL_ALLOC")
defstruct Url {

    init() {}

    def ToString(): String {
        @{
            Trap("STD_URL_URL_TO_STRING")
            PushResult(typeof String)
        }
    }

    impl Equality[Url,Url] {

        def Equals(lhs: Url, rhs: Url): Bool {
            @{
                Trap("STD_URL_EQUALITY_EQUALS")
                PushResult(typeof Bool)
            }
        }
    }
}


def ParseUrl(str: String): Url | Error {
    @{
        Trap("STD_PARSE_URL")
        PushResult(typeof Url | Error)
    }
}