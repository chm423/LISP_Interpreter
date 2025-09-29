(define caar (L) (car (car L)))
(define cadr (L) (car (cdr L)))
(define cddr(L) (cdr (cdr L)))
(define caddr (L) (car (cddr L)))
(define cdddr(L) (cdr (cddr L)))
(define list3 (a b c)
    (cons a (cons b (cons c ())))
)

;
;   MERGE SORT
;

(define mergesort (L)(cond
    ((nil? L) ())
    ((nil? (cdr L)) L)
    ('t (mergelevel (makelists L)))
))

(define makelists (L) (cond
    ((nil? L) ())
    ('t (cons
        (cons (car L) ())
        (makelists (cdr L))
    ))
))


(define mergelevel(LL)(cond
    ((nil? LL) ())
    ((nil? (cdr LL)) (car LL))
    ('t (mergelevel (mergepairs LL)))
))


(define mergepairs(LL)(cond
    ((nil? LL) ())
    ((nil? (cdr LL)) LL) ;only one list left
    ('t (cons (merge (car LL) (cadr LL)) (mergepairs (cddr LL)) ))
))


(define merge (L M)(cond
    ((nil? L) M)
    ((nil? M) L)
    ((lte (car L) (car M))
        (cons (car L) (merge (cdr L) M)))
    ('t  (cons (car M) (merge L (cdr M))))
))

'mergesort
(mergesort ())
(mergesort '(1))
(mergesort '(2 1))
(mergesort '(3 2 1 4 5 6 9 8 7))
(mergesort '(38 72 61 45 54 36 92 18 70 93 28 71 46 55 46 93 82 17))