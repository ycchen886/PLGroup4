import markovDSL._;

object Test extends MarkovDSL {
	def main(args: Array[String]) {
		OUTPUT (1 WORD) ABOUT "WALRUS"
		OUTPUT (10 SENTENCES) ABOUT "MARKOV CHAINS"
	}
}
