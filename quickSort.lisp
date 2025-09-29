(define caar (L) (car (car L)))
(define cadr (L) (car (cdr L)))
(define cddr(L) (cdr (cdr L)))
(define caddr (L) (car (cddr L)))
(define cdddr(L) (cdr (cddr L)))
(define list3 (a b c)
    (cons a (cons b (cons c ())))
)

;
;   QUICK SORT
;

;classic quicksort
(define quicksort(L)(cond
    ((nil? L) ())
    ((nil? (cdr L)) L)
    ('t (QSORT (partition (car L) (cdr L) () ()) ))
))

;partition L around p creating smaller and larger lists
(define partition(p L smaller larger)(cond
    ((nil? L) (list3 smaller p larger)) ;partitioning finished
    ((lte (car L) p)   ;first L goes to smaller list
        (partition p (cdr L) (cons (car L) smaller) larger))
    ('t ;first L goes to larger list
        (partition p (cdr L) smaller (cons (car L) larger)))
))

;given a partition break it up and call the helper
(define QSORT(P)
    (QKSORT (car P) (cadr P) (caddr P))
)

;quicksort the left and right partitions then rebuild the list
(define QKSORT(left p right)
    (append (quicksort left) (cons p (quicksort right)))
)

;given two list make one
(define append (L M)(cond
    ((nil? L) M)
    ('t (cons (car L) (append (cdr L) M)))
))

'quicksort
(quicksort ())
(quicksort '(1))
(quicksort '(2 1))
(quicksort '(3 2 1 4 5 6 9 8 7))
(quicksort '(38 72 61 45 54 36 92 18 70 93 28 71 46 55 46 93 82 17))