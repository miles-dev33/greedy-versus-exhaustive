///////////////////////////////////////////////////////////////////////////////
// maxprotein.hh
//
// Compute the set of foods that maximizes protein, within a calorie budget,
// with the greedy method or exhaustive search.
//
///////////////////////////////////////////////////////////////////////////////


#pragma once

#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

// One food item in the USDA database.
class Food {
private:
	// Human-readable description of the food, e.g. "all-purpose wheat
	// flour". Must be non-empty.
	std::string _description;

	// Human-readable description of the amount of the food in one
	// sample, e.g. "1 cup". Must be non-empty.
	std::string _amount;

	// Number of grams in one sample; must be non-negative.
	int _amount_g;

	// Energy, in units of kilocalories (commonly called "calories"), in
	// one sample; must be non-negative.
	int _kcal;

	// Number of grams of protein in one sample; most be non-negative.
	int _protein_g;

public:
	Food(const std::string& description,
		const std::string& amount,
		int amount_g,
		int kcal,
		int protein_g)
		: _description(description),
		_amount(amount),
		_amount_g(amount_g),
		_kcal(kcal),
		_protein_g(protein_g) {

		assert(!description.empty());
		assert(!amount.empty());
		assert(amount_g >= 0);
		assert(kcal >= 0);
		assert(protein_g >= 0);
	}

	const std::string& description() const { return _description; }
	const std::string& amount() const { return _amount; }
	int amount_g() const { return _amount_g; }
	int kcal() const { return _kcal; }
	int protein_g() const { return _protein_g; }

};

// Alias for a vector of shared pointers to Food objects.
typedef std::vector<std::shared_ptr<Food>> FoodVector;

// Load all the valid foods from a USDA database in their ABBREV
// format. Foods that are missing fields such as the amount string are
// skipped. Returns nullptr on I/O error.
std::unique_ptr<FoodVector> load_usda_abbrev(const std::string& path) {

	std::unique_ptr<FoodVector> failure(nullptr);

	std::ifstream f(path);
	if (!f) {
		return failure;
	}

	std::unique_ptr<FoodVector> result(new FoodVector);

	for (std::string line; std::getline(f, line); ) {

		std::vector<std::string> fields;
		std::stringstream ss(line);
		for (std::string field; std::getline(ss, field, '^'); ) {
			fields.push_back(field);
		}

		if (fields.size() > 53) {
			return failure;
		}

		std::string descr_field = fields[1],
			kcal_field = fields[3],
			protein_g_field = fields[4],
			amount_g_field = fields[48],
			amount_field = fields[49];

		auto remove_tildes = [](std::string& output,
			const std::string& field) {
			if ((field.size() < 3) ||
				(field.front() != '~') ||
				(field.back() != '~')) {
				return false;
			}
			else {
				output.assign(field.begin() + 1, field.end() - 1);
				return true;
			}
		};

		auto parse_mil = [](int& output, const std::string& field) {
			std::stringstream ss(field);
			double floating;
			ss >> floating;
			if (!ss) {
				return false;
			}
			else {
				output = lround(floating);
				return true;
			}
		};

		std::string description, amount;
		int amount_g, kcal, protein_g;
		if (remove_tildes(description, descr_field) &&
			remove_tildes(amount, amount_field) &&
			parse_mil(amount_g, amount_g_field) &&
			parse_mil(kcal, kcal_field) &&
			parse_mil(protein_g, protein_g_field)) {
			result->push_back(std::shared_ptr<Food>(new Food(description,
				amount,
				amount_g,
				kcal,
				protein_g)));
		}
	}

	f.close();
	
	return result;
}

// Convenience function to compute the total kilocalories and protein
// in a FoodVector. Those values are returned through the
// first two pass-by-reference arguments.
void sum_food_vector(int& total_kcal, 
	int& total_protein_g, 
	const FoodVector& foods) {
	total_kcal = total_protein_g = 0;
	for (auto& food : foods) {
		total_kcal += food->kcal();
		total_protein_g += food->protein_g();
	}
}

// Convenience function to print out each food in a FoodVector,
// followed by the total kilocalories and protein in it.
void print_food_vector(const FoodVector& foods) {
	for (auto& food : foods) {
		std::cout << food->description()
			<< " (100 g where each " << food->amount()
			<< " is " << food->amount_g() << " g)"
			<< " kcal=" << food->kcal()
			<< " protein=" << food->protein_g() << " g"
			<< std::endl;
	}

	int total_kcal, total_protein_g;
	sum_food_vector(total_kcal, total_protein_g, foods);
	std::cout << "total kcal=" << total_kcal
		<< " total_protein=" << total_protein_g << " g"
		<< std::endl;
}

// Filter the vector source, i.e. create and return a new FoodVector
// containing the subset of the foods in source that match given
// criteria. This is intended to 1) filter out foods with zero
// calories that are irrelevant to our optimization, and 2) limit the
// size of inputs to the exhaustive search algorithm since it will
// probably be slow. Each food that is included must have at least
// min_kcal kilocalories and at most max_kcal kilocalories. In
// addition, the the vector includes only the first total_size foods
// that match these criteria.
std::unique_ptr<FoodVector> filter_food_vector(const FoodVector& source,
	int min_kcal,
	int max_kcal,
	int total_size) {
	//Declaring a pointer to store the new vector in
	std::unique_ptr<FoodVector> result(new FoodVector);
	//Loads from source argument, the result vector of the proper food items with
	// the correct caloric intake specifications 
	for (auto& food : source) {
		if (food->kcal() > min_kcal 
			&& food->kcal() <= max_kcal 
			&& result->size() != total_size)
			result->push_back(food);
	}
	//Return the filtered food vector
	return result;
}

// Compute the optimal set of foods with a greedy
// algorithm. Specifically, among the food items that fit within a
// total_kcal calorie budget, choose the food whose protein is
// greatest. Repeat until no more foods can be chosen, either because
// we've run out of foods, or run out of calories.
std::unique_ptr<FoodVector> greedy_max_protein(const FoodVector& foods,
	int total_kcal) {
	FoodVector todo = foods;
	std::unique_ptr<FoodVector> result (new FoodVector);
	int result_cal = 0, index, counter;
	//Loops until we have run out of foods
	while (!todo.empty()) {
		index = 0;
		counter = 0;
		std::shared_ptr<Food> f (new Food(" "," ",0,0,0));
		//for each food in todo, food will become every 
		//single object in the vector
		for (auto& food : todo) {
			//finds the newest higher protein items 
			if (food->protein_g() > f->protein_g()) {
				f = food;
				index = counter;
			}
			counter++;
		}
		//End of the range which is the end of the list
		todo.erase(todo.begin() + index);
		//keeps track of our calorie budget 
		if (result_cal + f->kcal() <= total_kcal) {
			result->push_back(f);
			result_cal += f->kcal();
		}
	}
	return result;
}

// Compute the optimal set of foods with an exhaustive search
// algorithm. Specifically, among all subsets of foods, return the
// subset whose calories fit within the total_kcal budget, and whose
// total protein is greatest. To avoid overflow, the size of the foods
// vector must be less than 64.
std::unique_ptr<FoodVector> exhaustive_max_protein(const FoodVector& foods,
	int total_kcal) {
	//declares the n to the size of the food vector
	const int n = foods.size();
	assert(n < 64);
	//Declares the variables used within the sum food vector function
	int cand_calories, cand_protein, best_calories, best_protein;
	//Declares the best food vector unique pointer
	std::unique_ptr<FoodVector> best (new FoodVector);
	//Creates power to be 2^n
	double power = pow(2, n);
	//Subset generating nested for loop
	for (uint64_t bits = 0; bits < power; bits++) {
		std::unique_ptr<FoodVector> candidate (new FoodVector);
		for (int j = 0; j < n; j++) {
			//Line that generates subsets
			if (((bits >> j) & 1) == 1)
				//pushes all food items worthy of being a candidate
				candidate->push_back(foods[j]);
		}
		//Returns the total calories and protein for the input food vector
		sum_food_vector(cand_calories, cand_protein, *candidate);
		sum_food_vector(best_calories, best_protein, *best);
		//Updates the best vector with the optimal protein items 
		if (cand_calories <= total_kcal) 
			if (best->empty() || cand_protein > best_protein)
				*best = *candidate;
	}
	//Return the best subsets of protein items in the food vector for the given caloric bounds 
	return best;
}
