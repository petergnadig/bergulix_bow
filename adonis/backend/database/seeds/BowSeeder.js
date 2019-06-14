"use strict";

/*
|--------------------------------------------------------------------------
| BowSeeder
|--------------------------------------------------------------------------
|
| Make use of the Factory instance to seed database with dummy data or
| make use of Lucid models directly.
|
*/

/** @type {import('@adonisjs/lucid/src/Factory')} */
const Factory = use("Factory");

class BowSeeder {
  async run() {
    const bow = await Factory.model("App/Models/Bow").createMany(3);
  }
}

module.exports = BowSeeder;
