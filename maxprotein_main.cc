//
//#include "maxprotein.hh"
//#include "timer.hh"
//#include "cassert"
//#include <iostream>
//using namespace std;
//int main() {
//
//
//	int n = 6000;
//	auto all_foods = load_usda_abbrev("ABBREV.txt");
//    assert( all_foods );
//	//const std::unique_ptr<FoodVector> foodvector(new FoodVector);
//	auto foodvector = filter_food_vector(*all_foods, 1, 2500, n);
//	Timer timer;
//	auto output = greedy_max_protein(*foodvector, 2000);
//	double elapsed = timer.elapsed();
//	cout << "greedy_max_protein, "
//		<< "n=" << n << ", "
//		<< "elapsed time=" << elapsed << " seconds" << endl;
//		timer.reset();
//		
//		
//		int n = 25;
//		auto all_foods = load_usda_abbrev("ABBREV.txt");
//		assert(all_foods);
//		auto foodvector = filter_food_vector(*all_foods, 1, 2500, n);
//		Timer timer;
//		auto output = exhaustive_max_protein(*foodvector, 2000);
//		double elapsed = timer.elapsed();
//		cout << "exhuastive_max_protein, "
//			<< "n=" << n << ", "
//			<< "elapsed time=" << elapsed << " seconds" << endl;
//		timer.reset();
//
//
//
//		system("pause");
//  return 0;
//}
