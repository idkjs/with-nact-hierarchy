/* -  open Nact; and/or Nact.Operators;
   - call system to start it actor `let system = start()`
   - define type that actor will expect if not defined elsewhere

   - define actor function and call spawn type of function, spawnStateless for stateless actors
   or just spawn for spawnStateful.
   - 1st arg is optional name of actor as string, `~name="someActor"`
   - 2nd arg is unnamed reference to 'system'
   - 3rd arg is unnamed arg that takes a ref to a defined type that is expected, the "_" to
   call the actor function with follows after fat arrow. oCaml will infer the type if it can.
   - call the actor with dispatch(actor, input) or
   using Nact.Operator syntax `actor <-< { sometype: value }`
   - in stateful components, the first arg to the execution function is state,
    2nd is type ref, and the third is the context/ctx.
   */
/* define the message types/variants used between the api and actor system.  */
open Nact;

open Nact.Operators;

type contactId =
  | ContactId(int);

type contact = {
  name: string,
  email: string
};

/* type variant constructors to responses from actions we take on contacts like adding
   deleting, etc, if were successful then we will pass a value of that type to the success action */
type contactResponseMsg =
  | Success(contact)
  | NotFound;

/* type variant constructor for actions we will want to be able to do on contacts  */
type contactMsg =
  /* when we create a contact, we should pass the function the new contact values */
  | CreateContact(contact)
  /* when we delete, we should pass the contactID that we want to find and delete */
  | RemoveContact(contactId)
  /* we should pass the id to find it and the new info for the contact */
  | UpdateContact(contactId, contact)
  /* pass the id so we can find it */
  | FindContact(contactId);

/* describe shape of contact actors state using a ContactIdMap Module
   to hold the list of contacts */
module ContactIdCompare = {
  /* define the type req'd for this module */
  type t = contactId;
  /* define function to compare id's. Name the function, define the args it takes,
     then call the function on the passed in args */
  let compare = (ContactId(left), ContactId(right)) => compare(left, right);
};

/* define module, that when called takes the input from CIC and runs Map.Make on it.
   Map.Make is a Functor that builds an implementation of the map structure given a totally ordered type.
   see: https://caml.inria.fr/pub/docs/manual-ocaml/libref/Map.Make.html */
module ContactIdMap = Map.Make(ContactIdCompare);

/* create type to hold all contacts state and create sequential id for each
   contact monotically, see: https://en.wikipedia.org/wiki/Monotonic_function */
type contactsServiceState = {
  /* define contacts state type as ContactIdMap and pass in a contact value? */
  contacts: ContactIdMap.t(contact),
  seqNumber: int
};

/* each actor sends a message. Define functions to handle each message Variant we want
   to account for. When we call our actors we will use these functions.
   define functions using our types and variants to use in our NACT actor ()"s" */
/* createContact(): function of typecreates contact. args: takes object that has all of our contacts
   as one value and a seqNumber as the second value, a sender and a contact
   */
let createContact = ({contacts, seqNumber}, sender, contact) => {
  /* define contactid of type contactId as result of ContactID Variant
     which takes an int. Get contactId for this new item by passing the passed in sequence
     number to ContactId type, making sure its an int.  */
  let contactId = ContactId(seqNumber);
  /* use NACTO to call the target actor, here unnamed/passed in as a variable,
         and call the actor response function. Pass the expected type expected
         by contactId to the actor and the executing () is defined as Success from
         from our ContactResponseMsg Variants. If it works, we should get back
         the contact.
     */
  sender <-< (contactId, Success(contact));
  /* define 2nd arg expected type contacts to be passed to 3rd NACT actor arg
     which will have the newly added contact in the pool of all contacts usind the .add
      method on ContactIdMap avaibale via standard lib on Map.Make. See: https://caml.inria.fr/pub/docs/manual-ocaml/libref/Map.Make.html */
  let nextContacts = ContactIdMap.add(contactId, contact, contacts);
  /* 3rd NACT arg: state and type ref  */
  {contacts: nextContacts, seqNumber: seqNumber + 1}
};

/* define () to remove contact Variant. To remove a contact we need to know its contactId,
   then remove it from the contactIdMap. we always need state, a message sender and the contactId
   we want to remove. Need the state to know current state, which we will return after removing
   passed in contact id. Our object arg {contacts, seqNumber} has the two types our conctactsServiceState
   requires, sender is our actor... */
let removeContact = ({contacts, seqNumber}, sender, contactId) => {
  /* define new state var which processes removal of the passed in contactId by calling
     the Make.map.remove method */
  let nextContacts = ContactIdMap.remove(contactId, contacts);
  /* define what are NACTor message will be based on what the result of the Make.map.remove
     operation. Did it work or not? Send appropriate Success/NotFound Variant msg */
  let msg =
    /* check if ID is in state by calling nextContacts() */
    if (contacts === nextContacts) {
      /* if it is, define contact as result of Make.map.find, which will
         get the id from contacts via CIMap takes the id and all current contacts
         checks for it. We have not called it at this point, just getting the id since
         we know its thre because contacts === nextContacts */
      let contact = ContactIdMap.find(contactId, contacts);
      /* Call our Nactor here. Actor executor takes current state, type ref
         and context  */
      (contactId, Success(contact))
    } else {
      (contactId, NotFound)
    };
  /* call NACTor ref, the msg function */
  sender <-< msg;
  /* return this? */
  {contacts: nextContacts, seqNumber}
};

/* define function to handle updateContact message Variant/Constructor which takes two types
   as args, the contactId and a contact object with updated data. A NACT function takes
   1. state: contactServiceState, Nact.actorRef: sender, and the contactId and contact
   as required by the UpdateContact Variant.
   */
let updateContact = ({contacts, seqNumber}, sender, contactId, contact) => {
  /* define a () that defines new state as old state, use CIMap to find and remove the old version of this
     contactID then pipes in the passed in new state for that contact. */
  let nextContacts =
    ContactIdMap.remove(contactId, contacts) |> ContactIdMap.add(contactId, contact);
  /* () to get the msg we want the NActor to call */
  let msg =
    if (nextContacts === contacts) {
      (contactId, Success(contact))
    } else {
      (contactId, NotFound)
    };
  /* execute this Nactor if this () is called */
  sender <-< msg;
  /* NACTOR ref and type def */
  {contacts: nextContacts, seqNumber}
};

/* define () to handle FindContact Variant Constructor which takes contactId as arg. A stateful Nactor
   () require state: contactStateService, a Nactor. ref: sender, this actor needs a contactid to find the contact.
   */
let findContact = ({contacts, seqNumber}, sender, contactId) => {
  /* we dont have to change state here so no need to define the next state so we only need to create
     a () that produces a msg for finding the ID */
  let msg =
    try (contactId, Success(ContactIdMap.find(contactId, contacts))) {
    | Not_found => (contactId, NotFound)
    };
  /* define Nactor executor */
  sender <-< msg;
  /* dispatch actor message, pass type expected by actor function */
  {contacts, seqNumber}
};

/* put it all together to create the actor. Actor require open Nact, ref NACT system(), spawn, def ref, and call
   NACTO or dispatch */
let system = start();

/* define the actor which will execute with the above functions depending on which Variant we use */
let contactsService =
  /* optionally name the actor */
  spawn(
    ~name="contacts",
    /* ref system which we started */
    system,
    /* NACTO: pass in state, nactor ref and context using switch for each of our variants */
    (state, (sender, msg), _) =>
      (
        switch msg {
        /*  Call the CreateContact Variant, pass in contact actorRef,  context for each */
        | CreateContact(contact) => createContact(state, sender, contact)
        | RemoveContact(contactId) => removeContact(state, sender, contactId)
        | UpdateContact(contactId, contact) => updateContact(state, sender, contactId, contact)
        | FindContact(contactId) => findContact(state, sender, contactId)
        }
      )
      /* resolve state */
      |> Js.Promise.resolve,
    /* dispatch actor message */
    {contacts: ContactIdMap.empty, seqNumber: 0}
  );

let createErlich =
  query(
    ~timeout=100,
    contactsService,
    (tempReference) => (
      tempReference,
      CreateContact({name: "Erlich Bachman", email: "erlich@aviato.com"})
    )
  );

let createDinesh = (_) =>
  query(
    ~timeout=100,
    contactsService,
    (tempReference) => (
      tempReference,
      CreateContact({name: "Dinesh Chugtai", email: "dinesh@piedpiper.com"})
    )
  );

let findDinsheh = ((contactId, _)) =>
  query(~timeout=100, contactsService, (tempReference) => (tempReference, FindContact(contactId)));

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