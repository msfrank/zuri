
@@Plugin("/process")

import from "/collections" { Vector }

def _GetProgramId(): String {
    @{
        Trap("STD_PROCESS_GET_PROGRAM_ID")
        PushResult(typeof String)
    }
}

def _GetProgramMain(): Url {
    @{
        Trap("STD_PROCESS_GET_PROGRAM_MAIN")
        PushResult(typeof Url)
    }
}

def _GetArgument(index: Int): String {
    @{
        Trap("STD_PROCESS_GET_ARGUMENT")
        PushResult(typeof String)
    }
}

def _NumArguments(): Int {
    @{
        Trap("STD_PROCESS_NUM_ARGUMENTS")
        PushResult(typeof Int)
    }
}

definstance Process final {

    val ProgramId: String
    val ProgramMain: Url
    val Arguments: Vector[String]

    init {
        set this.ProgramId = _GetProgramId()
        set this.ProgramMain = _GetProgramMain()

        set this.Arguments = Vector[String]{}
        val numArguments: Int = _NumArguments()
        var curr: Int = 0
        while curr < numArguments {
            val argument: String = _GetArgument(curr)
            this.Arguments.Append(argument)
            set curr += 1
        }
    }
}