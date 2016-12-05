import markovDSL._;

object Test extends MarkovDSL {
	def main(args: Array[String]) {
		GENERATE (5 SENTENCES) ABOUT "programming" THEN OUTPUT
		GENERATE (1 LONG PARAGRAPH) ABOUT "love" LIKE TRUMP THEN OUTPUT
		GENERATE (1 LONG PARAGRAPH) ABOUT "wall" LIKE TRUMP THEN OUTPUT
	}
}
