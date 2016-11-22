package markovDSL;

import scala.language.postfixOps

class MarkovDSL {

	object OUTPUT {
		def apply(x: NumText) = {new ProgramState(amount = x.amount, textType = x.textType) }
	}

	case class NumText(var amount: Int, var textType: String)

	implicit class IntToNumText(amount: Int) {
		def WORD: NumText = { NumText(amount, "WORD") }
		def WORDS: NumText = { NumText(amount, "WORDS") }
		def SENTENCE: NumText = { NumText(amount, "SENTENCE") }
		def SENTENCES: NumText = { NumText(amount, "SENTENCES") }
	}

	class ProgramState(var amount: Int, var textType: String = null, var subject: String = "") {
		def ON(subject: String) = {this subject = subject; EXECUTE}
		def ABOUT(subject: String) = {this subject = subject; EXECUTE}
		def EXECUTE = {println(f"$amount $textType ABOUT $subject\n")}
	}
}