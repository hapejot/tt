Directory extend [
    |elements|
    subDirectory: name [
        Transcript show: 'new subdir:';
                    show: name;
                    nl.
        ^Directory new 
    ]

    init [ elements := OrderedCollection new ]

    add: file [
        elements isNil ifTrue: [ self init ].
        elements add: file.
        Transcript show: elements asString; nl
    ]
]

Object subclass: Executable [
    |name sources|
    name: aName [ name := aName ]
    name [ ^name ]

    init [ sources := OrderedCollection new ]
    addAll: someSources [ sources atAllPut: someSources ]

    Executable class >> named: name from: lst [
        ^(Executable new init)
                    addAll: lst;
                    yourself
    ]

    asString [ ^'*Executable*' ]
]
