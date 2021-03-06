/**
 *    Copyright (C) 2013 10gen Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Affero General Public License for more details.
 *
 *    You should have received a copy of the GNU Affero General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "mongo/db/jsobj.h"

namespace mongo {

    /** A range of values for one field. */
    struct Interval {

        // No BSONValue means we have to keep a BSONObj and pointers (BSONElement) into it.
        // 'start' may not point at the first field in _intervalData.
        // 'end' may not point at the last field in _intervalData.
        // 'start' and 'end' may point at the same field.
        BSONObj _intervalData;

        // Start and End must be ordered according to the index order.
        BSONElement start;
        bool startInclusive;

        BSONElement end;
        bool endInclusive;

        /** Creates an empty interval */
        Interval();

        string toString() const {
            stringstream ss;
            if (startInclusive) {
                ss << "[";
            }
            else {
                ss << "(";
            }
            // false means omit the field name
            ss << start.toString(false);
            ss << ", ";
            ss << end.toString(false);
            if (endInclusive) {
                ss << "]";
            }
            else {
                ss << ")";
            }
            return ss.str();
        }

        /**
         * Creates an interval that starts at the first field of 'base' and ends at the second
         * field of 'base'. (In other words, 'base' is a bsonobj with at least two elements, of
         * which we don't care about field names.)
         *
         * The interval's extremities are closed or not depending on whether
         * 'start'/'endIncluded' are true or not.
         */
        Interval(BSONObj base, bool startIncluded, bool endInclued);

        /** Sets the current interval to the given values (see constructor) */
        void init(BSONObj base, bool startIncluded, bool endIncluded);

        /** Returns true if an empty-constructed interval hasn't been init()-ialized yet */
        bool isEmpty() const;

        /**
         * Swap start and end points of interval.
         */
        void reverse();

        /** Returns how 'this' compares to 'other' */
        enum IntervalComparison {
            //
            // There is some intersection.
            //

            // The two intervals are *exactly* equal.
            INTERVAL_EQUALS,

            // 'this' contains the other interval.
            INTERVAL_CONTAINS,

            // 'this' is contained by the other interval.
            INTERVAL_WITHIN,

            // The two intervals intersect and 'this' is before the other interval.
            INTERVAL_OVERLAPS_BEFORE,

            // The two intervals intersect and 'this is after the other interval.
            INTERVAL_OVERLAPS_AFTER,

            //
            // There is no intersection.
            //

            INTERVAL_PRECEDES,

            // This happens if we have [a,b) [b,c]
            INTERVAL_PRECEDES_COULD_UNION,

            INTERVAL_SUCCEEDS,

            INTERVAL_UNKNOWN
        };

        IntervalComparison compare(const Interval& other) const;

        /**
         * toString for IntervalComparison
         */
        static string cmpstr(IntervalComparison c);

        /**
         * Updates 'this' with the intersection of 'this' and 'other'. If 'this' and 'other'
         * have been compare()d before, that result can be optionally passed in 'cmp'
         */
        void intersect(const Interval& other, IntervalComparison cmp = INTERVAL_UNKNOWN);

        /**
         * Updates 'this" with the union of 'this' and 'other'. If 'this' and 'other' have
         * been compare()d before, that result can be optionaly passed in 'cmp'.
         */
        void combine(const Interval& other, IntervalComparison cmp = INTERVAL_UNKNOWN);
    };

    inline bool operator==(const Interval& lhs, const Interval& rhs) {
        return lhs.compare(rhs) == Interval::INTERVAL_EQUALS;
    }

} // namespace mongo
