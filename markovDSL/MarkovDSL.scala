package markovDSL;

import sys.process._
import scala.language.postfixOps

class MarkovDSL {

	object GENERATE {
		def apply(x: NumText) = { new ProgramState(x) }
	}

	trait Type { var textType : String }

	object WORD extends Type { var textType = "WORD" }
	object WORDS extends Type { var textType = "WORD" }
	object SENTENCE extends Type { var textType = "SENTENCE" }
	object SENTENCES extends Type { var textType = "SENTENCE" }
	object PARAGRAPH extends Type { var textType = "PARAGRAPH" }
	object PARAGRAPHS extends Type { var textType = "PARAGRAPH" }


	case class NumText(var amount: Int, var textType: String, var length : String)

	implicit class IntToNumText(amount: Int) {
		def WORD : NumText = { new NumText(amount, "WORD", null) }
		def WORDS : NumText = { new NumText(amount, "WORD", null) }
		def SENTENCE : NumText = { new NumText(amount, "SENTENCE", null) }
		def SENTENCES : NumText = { new NumText(amount, "SENTENCE", null) }
		def PARAGRAPH : NumText = { new NumText(amount, "PARAGRAPH", null) }
		def PARAGRAPHS : NumText = { new NumText(amount, "PARAGRAPH", null) }
		def SHORT(t : Type) : NumText = { new NumText(amount, t.textType, "SHORT") }
		def LONG(t : Type) : NumText = { new NumText(amount, t.textType, "LONG") }
	}

	abstract sealed class OutputWord
	object OUTPUT extends OutputWord

	abstract sealed class TrumpWord
	object TRUMP extends TrumpWord

	class ProgramState(var numText : NumText, var subject: String = "", var trump: Boolean = false) {
		def ON(subject: String) = { this subject = subject; this }
		def ABOUT(subject: String) = { this subject = subject; this }
		def LIKE(trump: TrumpWord) = { this trump = true; this}
		def THEN(o: OutputWord) = {
			var commands = "";

			commands += f"-num ${numText.amount} "

			if (numText.length != null) {
				commands += f"-length ${numText.length} "
			}

			commands += f"-type ${numText.textType} "

			if (subject != "") {
				commands += f"-keywords $subject"
			}

			println(if (trump) (commands + " -trump") else commands)

			if (trump) {
				commands = f"./ngram/ngram data/Trump/all_Trump_uni.out data/Trump/all_Trump_bi.out data/Trump/all_Trump_bbi.out data/Trump/all_Trump_tri.out data/Trump/all_Trump_btri.out $commands"
			} else {
				commands = f"./ngram/ngram data/1_uni.out data/1_bi.out data/1_bbi.out data/1_tri.out data/1_btri.out $commands"
			}

            println(commands !!)
		}
	}
}
