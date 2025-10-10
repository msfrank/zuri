
defconcept IntoFlags[T] {

    decl ToValue(element: T): Int
}

defclass Flags[T] {

    val _intoFlags: IntoFlags[T]
    var _bits: Int

    init(using intoFlags: IntoFlags[T], initialSet: ...T) {
        set this._intoFlags = intoFlags
        set this._bits = 0
        for element: T in initialSet {
            this.Set(element)
        }
    }

    def Set(element: T) {
        val value: Int = this._intoFlags.ToValue[T](element)
        @{
            LoadData(this._bits)
            LoadData(value)
            BitwiseOr()
            StoreData(this._bits)
        }
    }

    def Unset(element: T) {
        val value: Int = this._intoFlags.ToValue[T](element)
        @{
            LoadData(this._bits)
            LoadData(value)
            BitwiseXor()
            StoreData(this._bits)
        }
    }

    def Contains(element: T): Bool {
        val value: Int = this._intoFlags.ToValue[T](element)
        var present: Int = 0
        @{
            LoadData(this._bits)
            LoadData(value)
            BitwiseAnd()
            StoreData(present)
        }
        present == 0 then false else true
    }

    def ToInt(): Int {
        this._bits
    }
}
