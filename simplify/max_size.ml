open Wcet_formula

let rec compute_size f =
	match f with
	| FConst (_,(wl,_)) -> (List.length wl) + 1
	| FParam _ -> 999999999
	| FPlus tl -> max tl
	| FUnion tl -> (sum tl) - (List.length tl) + 1
	| FPower (tb,te,_,_) | FPowerParam (tb,te,_,_) -> max [tb;te]
	| FAnnot (_,(_,n)) -> n+1
	| FProduct (_,t) | FBProduct (_,t) -> compute_size t

and max tl =
	match tl with
	| [] -> 0
	| x::xs ->
		let tmp_max = max xs
		and current = compute_size x in
			if current > tmp_max then
				current
			else
				tmp_max

and sum tl =
	match tl with
	| [] -> 0
	| x::xs ->
		let tmp_sum = sum xs
		and current = compute_size x in
			tmp_sum + current
