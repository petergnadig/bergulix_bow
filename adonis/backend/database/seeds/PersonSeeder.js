"use strict";

/*
|--------------------------------------------------------------------------
| PersonSeeder
|--------------------------------------------------------------------------
|
| Make use of the Factory instance to seed database with dummy data or
| make use of Lucid models directly.
|
*/

/** @type {import('@adonisjs/lucid/src/Factory')} */
const Factory = use("Factory");

class PersonSeeder {
  async run() {
    const bow = await Factory.model("App/Models/Person").createMany(3);
  }
}

module.exports = PersonSeeder;
