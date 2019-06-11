

let n = 134000000;

// see https://en.wikipedia.org/wiki/Primality_test
function isPrime(n) {
	if (n <= 3) {
		return n > 1;
	} else if ((n % 2) == 0 || (n % 3) == 0) {
		return false;
	}

	let i = 5;
	while ((i * i) <= n) {
		if ((n % i) == 0 || (n % (i + 2)) == 0) {
			return false;
		}


		i = i + 6;
	}

	return true;
}

function nextSmallerPrime(n){

	for(let i = n - 1; i >= 1; i--){
		if(isPrime(n)){
			return n;
		}
	}

	return 1;
}

function previousPrimeCongruent3mod4(number) {
	for (let i = number - 1; i >= 7; i--) {
		if ((i % 4) === 3 && isPrime(i)) {
			return i;
		}
	}

	return 7;
}

n = 100 * 1000 * 1000;
let prime = previousPrimeCongruent3mod4(n);

function permute(number, prime){
	if(number > prime){
		return number;
	}

	let residue = (number * number) % prime;

	if(number <= prime / 2){
		return residue;
	}else{
		return prime - residue;
	}
}

function createPermutations(prime){

	let indices = new Uint32Array(prime);

	for(let i = 0; i < prime; i++){
		let targetIndex = permute(i, prime);
		targetIndex = permute(targetIndex, prime);

		indices[i] = targetIndex;
	}

	return indices;
}

console.log("Verify that the method maps range [0, prime) to unique numbers that are in interval [0, prime)");
console.log("see: https://preshing.com/20121224/how-to-generate-a-sequence-of-unique-random-integers/");
console.log(`number: ${n}`);
console.log(`prime: ${prime}`);


//let permutations = createPermutations(prime);

//let part = new Array(...permutations.subarray(0, 20))
//console.log(`target indices[0, 20]: ${part.join(", ")}, ...`);

//let set = new Set(permutations);
//console.log(`#distinct target indices: ${set.size}`);

{

	let v = new Array(1000).fill(0);

	for(let i = 0; i < prime; i++){
		let targetIndex = permute(i, prime);
		//targetIndex = permute(targetIndex, prime);

		if(targetIndex < v.length){
			v[targetIndex] = v[targetIndex] + 1;
		}

		if(v[targetIndex] > 1){
			console.log(`${i} => ${targetIndex}`);
		}
	}

	console.log(v.join(", "));
	let max = Math.max(...v);
	let min = Math.min(...v);

	console.log(`max: ${max}`);
	console.log(`min: ${min}`);
}




