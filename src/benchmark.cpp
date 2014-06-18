/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2014 Marco Costalba, Joona Kiiski, Tord Romstad

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <algorithm>
#include <fstream>
#include <iostream>
#include <istream>
#include <vector>

#include "misc.h"
#include "position.h"
#include "search.h"
#include "thread.h"
#include "tt.h"
#include "uci.h"

using namespace std;

namespace {

const char* Defaults[] = {
  "8/1rnqknr1/1pppppp1/8/8/1PPPPPP1/1RNQKNR1/8 w - - 0 1",
  "8/2nkr3/1r3pp1/1p6/1N1P2P1/1P1K4/1R2R3/8 b - - 4 14",
  "8/1r1k2r1/3q1pp1/1p1n4/3PP1P1/1P1KQ3/1R4R1/8 w - - 0 12",
  "8/6r1/5Rp1/3k4/3P2P1/3K4/8/8 b - - 0 17",
  "8/1rnqkn2/1pp3r1/3pp1p1/1NP3P1/1P2PP2/1R1QKNR1/8 w - - 2 6",
  "8/3r2k1/1p2np2/3Q2p1/3n2P1/1P1N1P2/1R3KR1/8 b - - 0 14",
  "8/1r4r1/1pn1kp2/5Np1/2Pp2P1/1P1R1PK1/6R1/8 b - - 0 15",
  "8/8/5k2/1r6/3R1P2/6K1/8/8 b - - 0 43",
  "8/1rnqknr1/1p2ppp1/2pp4/2PP4/1P2PPP1/1RNQKNR1/8 w - - 0 3",
  "8/1rnqknr1/1p2p1p1/5p2/2PQ4/1P3PP1/1RN1KNR1/8 b - - 0 6",
  "8/5nr1/1N6/2k1p1p1/5pP1/1P1K1n2/1R4R1/8 w - - 0 19",
  "8/3r4/2kr4/2N1p1p1/4KpP1/5P2/6R1/8 w - - 0 21",
  "8/2r2nk1/4R3/5pp1/3K4/3N1PP1/8/8 b - - 0 22",
  "8/2r3k1/3R4/5pp1/3K4/3N1PP1/8/8 b - - 0 23",
  "8/2r3k1/3R4/6p1/3K1p2/3N1PP1/8/8 w - - 0 24",
  "8/8/8/3k4/4nK2/8/2R5/8 w - - 11 33",
  "8/2p5/8/2kPKp1p/2p4P/2P5/3P4/8 w - - 0 1",
  "8/1rnqknr1/1pppppp1/8/8/1PPPPPP1/1RNQKNR1/8 w - - 0 1",
  "8/2nkr3/1r3pp1/1p6/1N1P2P1/1P1K4/1R2R3/8 b - - 4 14",
  "8/1r1k2r1/3q1pp1/1p1n4/3PP1P1/1P1KQ3/1R4R1/8 w - - 0 12",
  "8/6r1/5Rp1/3k4/3P2P1/3K4/8/8 b - - 0 17",
  "8/1rnqkn2/1pp3r1/3pp1p1/1NP3P1/1P2PP2/1R1QKNR1/8 w - - 2 6",
  "8/3r2k1/1p2np2/3Q2p1/3n2P1/1P1N1P2/1R3KR1/8 b - - 0 14",
  "8/1r4r1/1pn1kp2/5Np1/2Pp2P1/1P1R1PK1/6R1/8 b - - 0 15",
  "8/8/5k2/1r6/3R1P2/6K1/8/8 b - - 0 43",
  "8/1rnqknr1/1p2ppp1/2pp4/2PP4/1P2PPP1/1RNQKNR1/8 w - - 0 3",
  "8/1rnqknr1/1p2p1p1/5p2/2PQ4/1P3PP1/1RN1KNR1/8 b - - 0 6",
  "8/5nr1/1N6/2k1p1p1/5pP1/1P1K1n2/1R4R1/8 w - - 0 19",
  "8/3r4/2kr4/2N1p1p1/4KpP1/5P2/6R1/8 w - - 0 21",
  "8/2r2nk1/4R3/5pp1/3K4/3N1PP1/8/8 b - - 0 22",

  // 5-man positions
  "8/8/8/8/5kp1/P7/8/1K1N4 w - - 0 1",     // Kc2 - mate
  "8/8/8/5N2/8/p7/8/2NK3k w - - 0 1",      // Na2 - mate
  "8/3k4/8/8/8/4B3/4KB2/2B5 w - - 0 1",    // draw

  // 6-man positions
  "8/8/1P6/5pr1/8/4R3/7k/2K5 w - - 0 1",   // Re5 - mate
  "8/2p4P/8/kr6/6R1/8/8/1K6 w - - 0 1",    // Ka2 - mate
  "8/8/3P3k/8/1p6/8/1P6/1K3n2 b - - 0 1",  // Nd2 - draw

  // 7-man positions
  "8/R7/2q5/8/6k1/8/1P5p/K6R w - - 0 124", // Draw
};

} // namespace

/// benchmark() runs a simple benchmark by letting Stockfish analyze a set
/// of positions for a given limit each. There are five parameters: the
/// transposition table size, the number of search threads that should
/// be used, the limit value spent for each position (optional, default is
/// depth 13), an optional file name where to look for positions in FEN
/// format (defaults are the positions defined above) and the type of the
/// limit value: depth (default), time in secs or number of nodes.

void benchmark(const Position& current, istream& is) {

  string token;
  Search::LimitsType limits;
  vector<string> fens;

  // Assign default values to missing arguments
  string ttSize    = (is >> token) ? token : "16";
  string threads   = (is >> token) ? token : "1";
  string limit     = (is >> token) ? token : "13";
  string fenFile   = (is >> token) ? token : "default";
  string limitType = (is >> token) ? token : "depth";

  Options["Hash"]    = ttSize;
  Options["Threads"] = threads;
  TT.clear();

  if (limitType == "time")
      limits.movetime = 1000 * atoi(limit.c_str()); // movetime is in ms

  else if (limitType == "nodes")
      limits.nodes = atoi(limit.c_str());

  else if (limitType == "mate")
      limits.mate = atoi(limit.c_str());

  else
      limits.depth = atoi(limit.c_str());

  if (fenFile == "default")
      fens.assign(Defaults, Defaults + 37);

  else if (fenFile == "current")
      fens.push_back(current.fen());

  else
  {
      string fen;
      ifstream file(fenFile.c_str());

      if (!file.is_open())
      {
          cerr << "Unable to open file " << fenFile << endl;
          return;
      }

      while (getline(file, fen))
          if (!fen.empty())
              fens.push_back(fen);

      file.close();
  }

  uint64_t nodes = 0;
  Search::StateStackPtr st;
  Time::point elapsed = Time::now();

  for (size_t i = 0; i < fens.size(); ++i)
  {
      Position pos(fens[i], Options["UCI_Chess960"], Threads.main());

      cerr << "\nPosition: " << i + 1 << '/' << fens.size() << endl;

      if (limitType == "perft")
          nodes += Search::perft<true>(pos, limits.depth * ONE_PLY);

      else
      {
          Threads.start_thinking(pos, limits, st);
          Threads.wait_for_think_finished();
          nodes += Search::RootPos.nodes_searched();
      }
  }

  elapsed = std::max(Time::now() - elapsed, Time::point(1)); // Avoid a 'divide by zero'

  dbg_print(); // Just before to exit

  cerr << "\n==========================="
       << "\nTotal time (ms) : " << elapsed
       << "\nNodes searched  : " << nodes
       << "\nNodes/second    : " << 1000 * nodes / elapsed << endl;
}
