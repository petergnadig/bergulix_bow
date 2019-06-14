'use strict';

/** @type {import('@adonisjs/lucid/src/Schema')} */
const Schema = use('Schema');

class ImuSchema extends Schema {
  up() {
    this.create('imus', table => {
      table.increments();

      table.string('serial', 10).notNullable();
      table.text('config', 10);

      table
        .integer('measurement_id')
        .unsigned()
        .references('id')
        .inTable('measurements');

      table.timestamps();
    });
  }

  down() {
    this.drop('imus');
  }
}

module.exports = ImuSchema;
