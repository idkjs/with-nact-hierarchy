/* define the message types/variants used between the api and actor system.  */
open Nact;

open Nact.Operators;

/* module using String Pervasive. See:https://caml.inria.fr/pub/docs/manual-ocaml/libref/Pervasives.html
   and https://caml.inria.fr/pub/docs/manual-ocaml/libref/String.html. A string is an immutable data structure
   that contains a fixed-length sequence of (single-byte) characters. Each character can be accessed in
   constant time through its index. */
module StringCompare = {
  type t = string;
  let compare = String.compare;
};

/*  Map.Make is a Functor that builds an implementation of the map structure given a totally ordered type.
       see: https://caml.inria.fr/pub/docs/manual-ocaml/libref/Map.Make.html
       In OCaml and Standard ML, a functor is a higher-order module (a module parameterized by one or more other modules),
    often used to define type-safe abstracted algorithms and data structures. Here the other modules are StringCompare and
    Map.Make.
       */
module StringMap = Map.Make(StringCompare);

type contactId =
  | ContactId(int);

/* describe shape of contact actors state using a ContactIdMap Module
   to hold the list of contacts */
module ContactIdCompare = {
  /* define the type req'd for this module */
  type t = contactId;
  /* define function to compare id's. Name the function, define the args it takes,
     then call the function on the passed in args */
  let compare = (ContactId(left), ContactId(right)) => compare(left, right);
};

/* define module, that when called takes the input from ContactIDCompare and runs Map.Make on it.
   Map.Make is a Functor that builds an implementation of the map structure given a totally ordered type.
   see: https://caml.inria.fr/pub/docs/manual-ocaml/libref/Map.Make.html */
module ContactIdMap = Map.Make(ContactIdCompare);

type contact = {
  name: string,
  email: string
};

type contactResponseMsg =
  | Success(contact)
  | NotFound;

type contactMsg =
  /* when we create a contact, we should pass the function the new contact values */
  | CreateContact(contact)
  /* when we delete, we should pass the contactID that we want to find and delete */
  | RemoveContact(contactId)
  /* we should pass the id to find it and the new info for the contact */
  | UpdateContact(contactId, contact)
  /* pass the id so we can find it */
  | FindContact(contactId);

/* create type to hold all contacts state and create sequential id for each
   contact monotically, see: https://en.wikipedia.org/wiki/Monotonic_function */
type contactsServiceState = {
  /* define contacts state type as ContactIdMap and pass in a contact value? */
  contacts: ContactIdMap.t(contact),
  seqNumber: int
};

let createContact = ({contacts, seqNumber}, sender, contact) => {
  let contactId = ContactId(seqNumber);
  sender <-< (contactId, Success(contact));
  let nextContacts = ContactIdMap.add(contactId, contact, contacts);
  {contacts: nextContacts, seqNumber: seqNumber + 1}
};

let removeContact = ({contacts, seqNumber}, sender, contactId) => {
  let nextContacts = ContactIdMap.remove(contactId, contacts);
  let msg =
    if (contacts === nextContacts) {
      let contact = ContactIdMap.find(contactId, contacts);
      (contactId, Success(contact))
    } else {
      (contactId, NotFound)
    };
  sender <-< msg;
  {contacts: nextContacts, seqNumber}
};

let updateContact = ({contacts, seqNumber}, sender, contactId, contact) => {
  let nextContacts =
    ContactIdMap.remove(contactId, contacts) |> ContactIdMap.add(contactId, contact);
  let msg =
    if (nextContacts === contacts) {
      (contactId, Success(contact))
    } else {
      (contactId, NotFound)
    };
  sender <-< msg;
  {contacts: nextContacts, seqNumber}
};

let findContact = ({contacts, seqNumber}, sender, contactId) => {
  let msg =
    try (contactId, Success(ContactIdMap.find(contactId, contacts))) {
    | Not_found => (contactId, NotFound)
    };
  sender <-< msg;
  {contacts, seqNumber}
};

/* - CREATE PARENT HIERARCHY
   - open Nact; and/or Nact.Operators;
      - call system to start it actor `let system = start()`
      - define type that actor will expect if not defined elsewhere
      - define parent actor function and call spawn type of function, spawnStateless for stateless actors
      or just spawn for spawnStateful. parent is stateful(?)
      - 1st arg is optional name of actor as string, `~name="someActor"`
      - 2nd arg is unnamed reference to 'parent'
      - 3rd arg is unnamed arg that takes a ref to a defined type that is expected, the "_" to
      call the actor function with follows after fat arrow. oCaml will infer the type if it can.
      - call the actor with dispatch(actor, input) or
      using Nact.Operator syntax `actor <-< { sometype: value }`
      - in stateful components, the first arg to the execution function is state,
       2nd is type ref, and the third is the context/ctx.
      */
/* define actor */
let system = start();

/* Create a parent to route requests/message to thh correct child,
   args are parent and child. Pass parent where you would pass system previosly
   */
let createContactsService = (parent, userId) =>
  spawn(
    ~name=userId,
    parent,
    /* state (always first in stateful), sender/message is ref to types this actor expects, _ calls the () in RML  */
    (state, (sender, msg), _) =>
      (
        switch msg {
        /* 3 params: state, ref to type and msg: here contact, to pass */
        | CreateContact(contact) => createContact(state, sender, contact)
        | RemoveContact(contactId) => removeContact(state, sender, contactId)
        | UpdateContact(contactId, contact) => updateContact(state, sender, contactId, contact)
        | FindContact(contactId) => findContact(state, sender, contactId)
        }
      )
      |> Js.Promise.resolve,
    /* dispatch using NACT.Operator, first arg is state: value, 2nd is  */
    {contacts: ContactIdMap.empty, seqNumber: 0}
  );

/* define parent contact service that checks if it has a child with the passed userId as they key,
   if it doesnt not, it spawns the child actor. Here we aren't passing a named actor. */
let contactsService =
  /* optionally name the actor */
  spawn
    /* ref system which we started */
    (
      system,
      /* Pass children as state so we can check the state for the current userId.
         2nd arg is the expected type and message we want to send.
         3rd, calls the function after we find the switch variant we want to use. */
      (children, (sender, userId, msg), ctx) => {
        let potentialChild =
          try (Some(StringMap.find(userId, children))) {
          | _ => None
          };
        Js.Promise.resolve(
          switch potentialChild {
          | Some(child) =>
            dispatch(child, (sender, msg));
            children
          | None =>
            let child = createContactsService(ctx.self, userId);
            dispatch(child, (sender, msg));
            StringMap.add(userId, child, children)
          }
        )
      },
      StringMap.empty
    );

/* test query functions */
let createErlich =
  query(
    ~timeout=100,
    contactsService,
    (tempReference) => (
      tempReference,
      "0",
      CreateContact({name: "Erlich Bachman", email: "erlich@aviato.com"})
    )
  );

let createDinesh = (_) =>
  query(
    ~timeout=100,
    contactsService,
    (tempReference) => (
      tempReference,
      "1",
      CreateContact({name: "Dinesh Chugtai", email: "dinesh@piedpiper.com"})
    )
  );

let findDinsheh = ((contactId, _)) =>
  query(
    ~timeout=100,
    contactsService,
    (tempReference) => (tempReference, "1", FindContact(contactId))
  );

let (>=>) = (promise1, promise2) => Js.Promise.then_(promise2, promise1);

createErlich
>=> createDinesh
>=> findDinsheh
>=> (
  (result) => {
    Js.log(result);
    Js.Promise.resolve()
  }
);