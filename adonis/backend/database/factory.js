"use strict";

/*
|--------------------------------------------------------------------------
| Factory
|--------------------------------------------------------------------------
|
| Factories are used to define blueprints for database tables or Lucid
| models. Later you can use these blueprints to seed your database
| with dummy data.
|
*/

/** @type {import('@adonisjs/lucid/src/Factory')} */
const Factory = use("Factory");

Factory.blueprint("App/Models/Person", (faker, i, data) => {
  return {
    name: data.name === undefined ? faker.name() : data.name
  };
});

Factory.blueprint("App/Models/Bow", (faker, i, data) => {
  return {
    description:
      data.description === undefined ? faker.word() : data.description
  };
});
