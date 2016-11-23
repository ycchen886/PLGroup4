package markovDSL;

import scala.language.postfixOps

class MarkovDSL {

	object GENERATE {
		def apply(x: NumText) = { new ProgramState(x) }
	}

	trait Type { var textType : String }

	object WORD extends Type { var textType = "WORDS" }
	object WORDS extends Type { var textType = "WORDS" }
	object SENTENCE extends Type { var textType = "SENTENCES" }
	object SENTENCES extends Type { var textType = "SENTENCES" }
	object PARAGRAPH extends Type { var textType = "PARAGRAPHS" }
	object PARAGRAPHS extends Type { var textType = "PARAGRAPHS" }


	case class NumText(var amount: Int, var textType: String, var length : String)

	implicit class IntToNumText(amount: Int) {
		def WORD : NumText = { new NumText(amount, "WORDS", null) }
		def WORDS : NumText = { new NumText(amount, "WORDS", null) }
		def SENTENCE : NumText = { new NumText(amount, "SENTENCES", null) }
		def SENTENCES : NumText = { new NumText(amount, "SENTENCES", null) }
		def PARAGRAPH : NumText = { new NumText(amount, "PARAGRAPHS", null) }
		def PARAGRAPHS : NumText = { new NumText(amount, "PARAGRAPHS", null) }
		def SHORT(t : Type) : NumText = { new NumText(amount, t.textType, "SHORT") }
		def LONG(t : Type) : NumText = { new NumText(amount, t.textType, "LONG") }
	}

	abstract sealed class OutputWord
	object OUTPUT extends OutputWord

	class ProgramState(var numText : NumText, var subject: String = "") {
		def ON(subject: String) = { this subject = subject; this }
		def ABOUT(subject: String) = { this subject = subject; this }
		def THEN(o: OutputWord) = { println(f"${numText.amount} ${numText.length} ${numText.textType} ABOUT $subject\n") }
	}
}