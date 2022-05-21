Object subclass: Test [
    |d|

    Test class >> new [
        |r|
        r := super new.
        r init.
        ^r
    ]

    init [
        Transcript show: 'Init'; nl.
        d := Dictionary new.
        d   at: 1 put: 'Eins';
            at: 2 put: 'Zwei';
            at: 3 put: 'Drei';
            at: #eins put: 1;
            at: #zwei put: 2;
            at: #drei put: 3.
    ]

    absent [ ^'absent' ]
    test: aKey [
       ^d at: aKey ifAbsent: [d at: aKey put: '#new'. ]
    ]
]


Transcript show: 'Starting test'; nl.
x := Test new.

Transcript  show: '1: '; show: (x test: 1); nl;
            show: '4: '; show: (x test: 4); nl.
