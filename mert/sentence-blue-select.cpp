#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <fstream>

#include "BleuScorer.h"

using namespace std;
using namespace MosesTuning;

int main(int argc, char **argv)
{
  if (argc == 1 || argc > 2) {
    cerr << "Usage: ./sentence-bleu-select ref < candidate > selected" << endl;
    return 1;
  }

  vector<string> refFiles(argv + 1, argv + argc);

  // TODO all of these are empty for now
  string config;
  string factors;
  string filter;

  BleuScorer scorer(config);
  scorer.setFactors(factors);
  scorer.setFilter(filter);
  scorer.setReferenceFiles(refFiles);

  string line;
  //load reference lines into memory
  vector<string> reflines;
  ifstream ifs(argv[1]);
  while (getline(ifs, line)) {
	  reflines.push_back(line);
  }
  ifs.close();

  if (reflines.size() != scorer.GetReferences().size()) {
	  cerr<<endl<<"Loaded ref size mismatch"<<endl;
	  return 1;
  }
  cerr<<endl<<"Total reference loaded: "<<scorer.GetReferences().size()<<endl;

  set<size_t> selectedIds;
  double totalScore = 0;
  size_t line_count = 0;
  while (getline(cin, line)) {
	//preparing statistics
	ScoreStats scoreentry;
	float maxScore = -1;
	int curIndex = -1;
	for (size_t i = 0; i < scorer.GetReferences().size(); i++) {
		if (selectedIds.find(i) != selectedIds.end()) { //find one already
			continue;
		}
		scorer.prepareStats(i, line, scoreentry);
		vector<float> stats(scoreentry.getArray(), scoreentry.getArray() + scoreentry.size());
		float score = BleuScorer::sentenceLevelBleuPlusOne(stats);
		if (score > maxScore) {
			maxScore = score;
			curIndex = i;
		}
	}
	if (curIndex < 0 || curIndex >= (int)scorer.GetReferences().size()) {
		cerr<<endl<<"Cannot find candidate for: "<<line<<endl;
		return 1;
	}
	selectedIds.insert((size_t)curIndex);
	cerr<<curIndex<<"\t"<<maxScore<<endl;
	cout<<reflines[curIndex]<<endl;
	totalScore += maxScore;
	line_count ++;

	//if (line_count % 30 == 0) cerr<<"\r/ "<<line_count;
	//else if (line_count % 20 == 0) cerr<<"\r- "<<line_count;
	//else if (line_count % 10 == 0) cerr<<"\r\\ "<<line_count;
  }

  cerr<<endl<<"Total lines: "<<line_count<<endl;
  cerr<<"Average BLEU score: "<<totalScore / line_count<<endl;

  return 0;
}
