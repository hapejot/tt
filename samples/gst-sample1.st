Object subclass: Account [
    | balance |
    <comment: 'I represent a place to deposit and withdraw money'>
    Account class >> new [
        <category: 'instance creation'>
        | r |
        r := super new.
        r init.
        ^r
    ]
    init [
        <category: 'initialization'>
        balance := 0
    ]
]
