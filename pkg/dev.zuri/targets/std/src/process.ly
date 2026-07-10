
@@Plugin("/std")


import from "/collections" { Vector }
import from "/url" { ParseUrl, Url }


def _GetProgramId(): String {
    @{
        Trap("STD_PROCESS_GET_PROGRAM_ID")
        PushResult(typeof String)
    }
}

def _GetProgramMain(): Url {
    var programMain: String = ""
    @{
        Trap("STD_PROCESS_GET_PROGRAM_MAIN")
        PushResult(typeof String)
        StoreData(programMain)
    }
    match ParseUrl(programMain) {
        when url: Url -> url
        else          -> Url{}
    }
}

def _GetArgument(index: I64): String {
    @{
        Trap("STD_PROCESS_GET_ARGUMENT")
        PushResult(typeof String)
    }
}

def _NumArguments(): I64 {
    @{
        Trap("STD_PROCESS_NUM_ARGUMENTS")
        PushResult(typeof I64)
    }
}

definstance Process final {

    val ProgramId: String
    val ProgramMain: Url
    val Arguments: Vector[String]

    init() {
        this.ProgramId = _GetProgramId()
        this.ProgramMain = _GetProgramMain()
        this.Arguments = Vector[String]{}

        val numArguments: I64 = _NumArguments()
        var curr: I64 = 0
        while curr < numArguments {
            val argument: String = _GetArgument(curr)
            this.Arguments.Append(argument)
            curr += 1
        }
    }
}