DROP TABLE accomodation;
DROP TABLE payment;
DROP TABLE room;
DROP TABLE reservation;
DROP TABLE customer;
DROP TABLE receptionist;

--ROOM
CREATE TABLE room (
  "number" INT NOT NULL PRIMARY KEY,
  availableness VARCHAR(80) NOT NULL,
  type VARCHAR(80) NOT NULL,
  price FLOAT NOT NULL,
  number_of_beds INT NOT NULL
);

--CUSTOMER
CREATE TABLE customer (
  id INT GENERATED AS IDENTITY NOT NULL PRIMARY KEY,
  name VARCHAR(80) NOT NULL,
  birthdate DATE NOT NULL,
  phone_number VARCHAR(12) NOT NULL,
  address VARCHAR(80) NOT NULL,
  email VARCHAR(80) NOT NULL
);

--RECEPTION
CREATE TABLE receptionist (
  id INT GENERATED AS IDENTITY NOT NULL PRIMARY KEY,
  name VARCHAR(80) NOT NULL,
  birthdate DATE NOT NULL,
  phone_number VARCHAR(12) NOT NULL,
  address VARCHAR(80) NOT NULL,
  email VARCHAR(80) NOT NULL
);

--REZERVATION
CREATE TABLE reservation (
  id_reservation INT GENERATED AS IDENTITY NOT NULL PRIMARY KEY,
  user_id INT NOT NULL ,
  CONSTRAINT customer_id_fk 
    FOREIGN KEY (user_id) REFERENCES customer (id)
      ON DELETE CASCADE,
      id_receptionist INT NOT NULL ,
  CONSTRAINT receptionist_id_fk 
    FOREIGN KEY (id_receptionist) REFERENCES receptionist (id)
      ON DELETE CASCADE,
   check_in DATE NOT NULL,
   check_out DATE NOT NULL,
   number_of_people INT NOT NULL,
   number_of_rooms INT NOT NULL,
   serv_req VARCHAR(80) NOT NULL,
   state VARCHAR(80) NOT NULL
  
);

--PAYING 
CREATE TABLE payment (
  payment_id INT GENERATED AS IDENTITY NOT NULL PRIMARY KEY,
  user_id INT NOT NULL ,
  CONSTRAINT payment_id_fk 
    FOREIGN KEY (user_id) REFERENCES customer (id)
      ON DELETE CASCADE,
       id_reservation INT NOT NULL ,
  CONSTRAINT reservation_id_fk 
    FOREIGN KEY (id_reservation) REFERENCES reservation (id_reservation)
      ON DELETE CASCADE,
   amount INT NOT NULL,
   type VARCHAR(20) NOT NULL
);

--LENGTH OF STAY
CREATE TABLE accomodation (
  accomodation_id INT GENERATED AS IDENTITY NOT NULL PRIMARY KEY,
  reservation_id  INT NOT NULL, 
  CONSTRAINT reserv_id_fk 
   FOREIGN KEY (reservation_id) REFERENCES reservation (id_reservation)
     ON DELETE CASCADE,
  number_room INT NOT NULL,
  CONSTRAINT number_room_fk 
    FOREIGN KEY (number_room) REFERENCES room ("number")
      ON DELETE CASCADE,
   check_in DATE NOT NULL,
   check_out DATE NOT NULL,
   number_of_people INT NOT NULL,
   number_of_rooms INT NOT NULL,
   state VARCHAR(80) NOT NULL

);


----------------------------FILLING DATABASE TABLES----------------------------

--CUSTOMER
INSERT INTO customer (name, birthdate, phone_number, address, email)
VALUES (
  'Yaqub Turner',
  TO_DATE('1993-03-21', 'yyyy/mm/dd'),
  '4201111111', 'Bauerova 4, Brno',
  'turner@gmail.com'
);

INSERT INTO customer (name, birthdate, phone_number, address, email)
VALUES (
  'Dan Pokorny',
  TO_DATE('1996-12-11', 'yyyy/mm/dd'),
  '4201111112', 'Technicka 6, Brno',
  'pokr@gmail.com'
);

INSERT INTO customer (name, birthdate, phone_number, address, email)
VALUES (
 'Shana Snider',
  TO_DATE('1981-05-29', 'yyyy/mm/dd'),
  '4201111113', 'Beethovenova 44, Brno',
  'snider@gmail.com'
);

--RECEPTIONIST
INSERT INTO receptionist (name, birthdate , phone_number, address, email)
VALUES (
  'Adrienne Merritt',
  TO_DATE('1993-07-29', 'yyyy/mm/dd'), 
  '4201111114', 'Brandlova 55, Brno',
  'errit@gmail.com'
);

--RESEVATION
INSERT INTO reservation (user_id, id_receptionist, check_in, check_out, number_of_people, number_of_rooms, serv_req, state)
VALUES (
  1,
  1,
  TO_DATE('2021/04/01', 'yyyy/mm/dd'),
  TO_DATE('2021/04/10', 'yyyy/mm/dd'),
  2,
  1,
  'must contain at least three chairs',
  'canceled'
);

INSERT INTO reservation (user_id, id_receptionist, check_in, check_out, number_of_people, number_of_rooms, serv_req, state)
VALUES (
  2,
  1,
  TO_DATE('2022/04/01', 'yyyy/mm/dd'),
  TO_DATE('2022/04/10', 'yyyy/mm/dd'),
  4,
  2,
  'null', 
  'paid'
);

--PAYMENT
INSERT INTO payment (user_id, id_reservation, amount, type)
VALUES (
  1,
  1,
  500, 
  'card'
);

INSERT INTO payment (user_id, id_reservation, amount, type)
VALUES (
  2,
  2,
  1000,
  'cash'
);

--ROOM
INSERT INTO room ("number", availableness, type, price, number_of_beds)
VALUES (
  4,
  'available',
  '1 beds',
  350,
  1
);

INSERT INTO room ("number", availableness, type, price, number_of_beds)
VALUES (
  5,
  'available',
  '2 bed',
  700,
  2
);

--ACCOMDATION
INSERT INTO accomodation (reservation_id, number_room, check_in, check_out, number_of_people, number_of_rooms, state)
VALUES (
  1,
  4,
  TO_DATE('2021/04/01', 'yyyy/mm/dd'),
  TO_DATE('2021/04/10', 'yyyy/mm/dd'),
  2,
  1,
  'ended'
);

INSERT INTO accomodation (reservation_id, number_room, check_in, check_out, number_of_people, number_of_rooms, state)
VALUES (
  2,
  5,
  TO_DATE('2022-04-01', 'yyyy/mm/dd'),
  TO_DATE('2022-04-10', 'yyyy/mm/dd'),
  3,
  2,
  'current'
);

