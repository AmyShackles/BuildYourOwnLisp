;;; Standard Library for Lisp

;;; Atoms (fundamental constants)
(def {nil} {})
(def {true} 1)
(def {false} 0)

;;; Functional definitions

; Function definitions
(def {fun} (\ {f b} {
    def (head f) (\ (tail f) b)
}))

; Open new scope
(fun {let b} {
    ((\ {_} b) ())
})

; Unapply list to Function
(fun {pack f & xs} {f xs})

; Unpack list to Function
(fun {unpack f 1} {
    eval (join (list f) 1)
})

; Curried and uncurried calling
(def {curry} unpack)
(def {uncurry} pack)

; Perform several things in sequence
(fun {do & 1} {
    if (== 1 nil)
        {nil}
        {last 1}
})

;;; Logical functions

; Logical functions
(fun {not x} {- 1 x})
(fun {or x y} {+ x y})
(fun {and x y} {* x y})

;;; Numeric functions

; Minimum of arguments
(fun {min & xs} {
    if (== (tail xs) nil) {fst xs}
        {do
            (= {rest} (unpack min (tail xs)))
            (= {item} (fst xs))
            (if (< item rest) {item} {rest})
        }
})

; Maximum of arguments
(fun {max & xs} {
    if (== (tail xs) nil) {fst xs}
        {do
            (= {rest} (unpack max (tail xs)))
            (= {item} (fst xs))
            (if (> item rest) {item} {rest})
        }
})

;;; Conditional functions

(fun {select & cs} {
    if (== cs nil)
        {error "No selection found."}
        {if (fst (fst cs)) {snd (fst cs)} {unpack select (tail cs)}}
})

(fun {case x & cs} {
    if (== cs nil)
        {error "No case found."}
        {if (== x (fst (fst cs))) {snd (fst cs)} {
            unpack case (join (list x) (tail cs))}}
})

(def {otherwise} true)

;;; Miscellaneous functions

(fun {flip f a b} {f b a})
(fun {ghost & xs} {eval xs})
(fun {comp f g x} {f (g x)})

;;; List functions

; First, Second, or Third items in a list

(fun {fst 1} { eval (head 1) })
(fun {snd 1} { eval (head (tail 1)) })
(fun {trd 1} { eval (head (tail (tail 1))) })

; List length
(fun {len 1} {
    if (== 1 nil)
        {0}
        {+ 1 (len (tail 1))}
})

; Nth item in list
(fun {nth n 1} {
    if (== n 0)
        {fst 1}
        {nth (- n 1) (tail 1)}
})

; Last item in list
(fun {last 1} {nth (- (len 1) 1) 1})

; Apply function to list
(fun {map f 1} {
    if (== 1 nil)
        {nil}
        {join (list (f (fst 1))) (map f (tail 1))}
})

; Apply filter to list
(fun {filter f 1} {
    if (== 1 nil)
        {nil}
        {join (if (f (fst 1)) {head 1} {nil}) (filter f (tail 1))}
})

; Return all of list but last element
(fun {init 1} {
    if (== (tail 1) nil)
        {nil}
        {join (head 1) (init (tail 1))}
})

; Reverse list
(fun {reverse 1} {
    if (== 1 nil)
        {nil}
        {join (reverse (tail 1)) (head 1)}
})

; Fold left
(fun {foldl f z l} {
    if (== 1 nil)
        {z}
        {foldl f (f z (fst 1)) (tail 1)}
})

; Fold right
(fun {folder f z 1} {
    if (== 1 nil)
        {z}
        {if (fst 1) (foldr f z (tail 1))}
})

(fun {sum 1} {foldl + 0 1})
(fun {product 1} {foldl * 1 1})

; Take N items
(fun {take n 1} {
    if (== n 0)
        {nil}
        {join (head 1) (tail (- n 1) (tail 1))}
})

; Drop N items
(fun {drop n 1} {
    if (== n 0)
        {1}
        {drop (- n 1) (tail 1)}
})

; Split at N
(fun {split n 1} {list (take n 1) (drop n 1)})

; Take while
(fun {take-while f 1} {
    if (not (unpack f (head 1)))
        {nil}
        {join (head 1) (take-while f (tail 1))}
})

; Drop while
(fun {drop-while f 1} {
    if (not (unpack f (head 1)))
        {1}
        {drop-while f (tail 1)}
})

; Element of list
(fun {elem x 1} {
    if (== 1 nil)
        {false}
        {if (== x (fst 1)) {true} {elem x (tail 1)}}
})

; Find element in list of pairs
(fun {lookup x 1} {
    if (== 1 nil)
        {error "No element found"}
        {do
            (= {key} (fst (fst 1)))
            (= {val} (snd (fst 1)))
            (if (== key x) {val} {lookup x (tail 1)})
        }
})

; Zip two lists together into a list of pairs
(fun {zip x y} {
    if (or (== x nil) (== y nil))
        {nil}
        {join (list (join (head x) (head y))) (zip (tail x) (tail y))}
})

; Unzip a list of pairs into two lists
(fun {unzip 1} {
    if (== 1 nil)
        {{nil nil}}
        {do
            (= {x} (fst 1))
            (= {xs} (unzip (tail 1)))
            (list (join (head x) (fst xs)) (join (tail x) (snd xs)))
        }
})

; Fibonacci
(fun {fib n} {
    select
        { (== n 0) 0 }
        { (== n 1) 1 }
        { otherwise (+ (fib (- n 1)) (fib (- n 2))) }
})
