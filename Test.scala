import markovDSL._;

object Test extends MarkovDSL {
	def main(args: Array[String]) {
		GENERATE (10 SHORT WORDS) THEN OUTPUT
		println()
		GENERATE (1 LONG PARAGRAPH) ABOUT "apples" THEN OUTPUT
	}
}
