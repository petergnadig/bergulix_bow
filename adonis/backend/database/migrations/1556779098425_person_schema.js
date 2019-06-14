'use strict';

/** @type {import('@adonisjs/lucid/src/Schema')} */
const Schema = use('Schema');

class PersonSchema extends Schema {
  up() {
    this.create('persons', table => {
      table.increments();
      table.string('name', 45).notNullable();
      table.timestamps();
    });
  }

  down() {
    this.drop('persons');
  }
}

module.exports = PersonSchema;
