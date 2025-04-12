namespace GroverHashCracker {
    open Microsoft.Quantum.Intrinsic;
    open Microsoft.Quantum.Canon;

    operation FakeHash(qs : Qubit[]) : Unit is Adj {
        X(qs[0]);
        X(qs[2]);
    }

    operation Oracle(qs : Qubit[], target : Bool[], hash_fn : (Qubit[] => Unit is Adj)) : Unit is Adj {
        hash_fn(qs);

        for (i in 0 .. Length(qs) - 1) {
            if not target[i] {
                X(qs[i]);
            }
        }

        Controlled Z(Most(qs), Tail(qs));

        for (i in 0 .. Length(qs) - 1) {
            if not target[i] {
                X(qs[i]);
            }
        }

        Adjoint hash_fn(qs);
    }
}
