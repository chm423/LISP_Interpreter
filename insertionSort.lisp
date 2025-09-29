(define caar (L) (car (car L)))
(define cadr (L) (car (cdr L)))
(define cddr(L) (cdr (cdr L)))
(define caddr (L) (car (cddr L)))
(define cdddr(L) (cdr (cddr L)))
(define list3 (a b c)
    (cons a (cons b (cons c ())))
)

;
;   INSERTION SORT
;

(define insertionsort(L) (INSERTSORT L () ))

(define INSERTSORT(unsorted sorted)(cond
    ((nil? unsorted) sorted)
    ('t  (INSERTSORT
            (cdr unsorted)
            (insert (car unsorted) sorted)
        ))
))

(define insert(x L)(cond
    ((nil? L) (cons x ())); tail
    ((lte x (car L)) (cons x L)); head
    ('t  (cons (car L) (insert x (cdr L)))); middle
))


'insertionsort
(insertionsort ())
(insertionsort '(1))
(insertionsort '(2 1))
(insertionsort '(3 2 1 4 5 6 9 8 7))
(insertionsort '(38 72 61 45 54 36 92 18 70 93 28 71 46 55 46 93 82 17))