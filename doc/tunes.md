## Trek14 Tunes

Music for Trek14 on the trs-80 is produced by wiggling the cassette port (255).

To make transcription of music, at least vaguely, possible the *playNotes* function takes a melody described as a character sequence.

The format of the sequence is as follows:

* A-G
  
  Signify notes within one octave.

* #

  May follow a note code to indicate sharp. eg G#

* b
  
  Indicates a flat, eg Bb

* *number*

  Indicates length of the note (if different from the previous one), Eg A4, G#2.

* +
  
  Go up one octave. The start octave contains middle C. Note that the + applies to the note immediately *before* it. eg A4+

* -

  Go down one octave. Applies to the note immediately before it.

* *number* t

  Set tempo. The standard tempo is 12. eg 12t, but a faster tempo might be 16t, for example. Usually, this option is put right at the start.


Example tune:

    playNotes("C4DECCDECEFG8E4FG8G2AGFE4CG2AGFE4CDG-C+CDG-C+C");








